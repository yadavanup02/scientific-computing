reset
reset session
set colorsequence podo

set datafile sep '\t'
set datafile columnheaders
set xdata time
set timefmt "%d.%m.%Y %H:%M:%S"
# set decimal locale 'de_DE.UTF-8'
#set decimalsign ','
#set decimalsign locale
### Paths - To be changed every time
########################
# pathgas = "/net/files/share/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/GastargetLog/2606"
path_apr = "/net/files/share/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/PelletronLog/2505"
# path_may = "/net/files/share/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/PelletronLog/2504"

### Filenames - To be changed every time
########################
#LISTgas = system("ls ".pathgas."/Log_250511_*.csv")
LISTgas = system("ls ".path_apr."/Log_250502_*.csv ".path_apr."/Log_250503_*.csv")

### Date and time - To be changed every time
#######################
start = strptime("%d.%m.%Y %H:%M:%S", "02.05.2025 17:30:00")
stop  = strptime("%d.%m.%Y %H:%M:%S", "02.05.2025 21:00:00")
Run_number = 4462

### Temporary data files
temp = "temp.txt"
GVM_tv = "GVM-TV-Fehler.txt" 
TV = "TV.txt"

### Create output directory for the run
output_dir = sprintf("GVM-TV/Run%d", Run_number)
system(sprintf("mkdir -p %s", output_dir))

# set table temp
#     plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.7f",column(58))):(sprintf("%.7f",column(59))) with table
# unset table

# Step 1: Use Linux sed to swap commas for dots dynamically in the file stream
set table temp
    plot for [data in LISTgas] "< sed 's/,/./g' ".data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.7f",column(58))):(sprintf("%.7f",column(59))) with table
unset table
# Step 2: Filter GVM-TV data between start and stop
set print GVM_tv
    print "time GVM-TV Fehler [kV]"
unset print
set xdata
set table GVM_tv
    plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)) with table if ($2>start&&$2<stop)
unset table

# Step 3: Filter TV data between start and stop
set print TV
    print "time TV[kV]"
unset print
set xdata
set table TV
    plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$4)) with table if ($2>start&&$2<stop)
unset table
# set print GVM_tv
#     print "time GVM-TV Fehler [V]"
# unset print
# set xdata
# set table GVM_tv
#     plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3*1000)) with table if ($2>start&&$2<stop)
# unset table

# set print TV
#     print "time TV[kV]"
# unset print
# set xdata
# set table TV
#     plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$4)) with table if ($2>start&&$2<stop)
# unset table

# print "\n=== First 10 Data Points: GVM-TV Fehler ==="
# system("head -n 11 GVM-TV-Fehler.txt")

# print "\n=== First 10 Data Points: TV ==="
# system("head -n 11 TV.txt")
### Get statistical metrics from the temporary files
set xdata
stats TV u 1:2 skip 1 nooutput
TV_ave = STATS_mean_y

stats GVM_tv u 1:2 nooutput
GVM_avg = STATS_mean_y
# uncertainity in the average value 
GVM_avg_unce  = STATS_stddev_y / sqrt(STATS_records)

# Calculate the true RMS using the mean and standard deviation
stats GVM_tv u 1:2 nooutput
GVM_rms = sqrt(STATS_mean_y**2 + STATS_stddev_y**2)

stats GVM_tv u 1:2 nooutput
GVM_std = STATS_stddev_y

# Calculate the Mean Absolute Error (MAE)
stats GVM_tv u 1:(abs($2)) skip 1 nooutput
GVM_mae = STATS_mean_y

### Calculate duration in seconds
duration_seconds = stop - start

# Print calculated metrics to the terminal console
print "\n============================================="
print sprintf("  Run Number:          %d", Run_number)
print sprintf("  Duration:            %d seconds", duration_seconds)
print "============================================="
# print sprintf("  Average TV:          %.5f kV", TV_ave)
# print "---------------------------------------------"
# print sprintf("  GVM-TV Average:      %.5f V", GVM_avg)
print sprintf("  GVM-TV Std Dev (σ):  %.3f V", GVM_std)
print sprintf("  GVM-TV RMS:          %.3f V", GVM_rms)
print sprintf("  GVM-TV unc:          %.3f V", GVM_avg_unce)

# print sprintf("  GVM-TV MAE:          %.5f V", GVM_mae)
print "=============================================\n"

### Setup plotting terminal for headlessly saving the PNG file
set xdata time
set timefmt "%s"
set terminal pngcairo enhanced size 1200,800
set output sprintf("%s/GVM-TV-Fehler_Run%d.png", output_dir, Run_number)

set multiplot layout 2,1

# Top Panel: GVM-TV Fehler displaying both tracking indicators
set ylabel "GVM-TV Fehler [V]"
set yrange [-2.0 : 2.0]
set format x ""
unset xlabel
plot GVM_tv u 1:2 t "GVM-TV Fehler [kV]" pt 6, \
     GVM_rms title "RMS = ".sprintf("%.3f",GVM_rms)." kV", \
     GVM_mae title "MAE = ".sprintf("%.3f",GVM_mae)." kV", \
    GVM_avg title "AVG = ".sprintf("%.3f", GVM_avg)." kV" ,\
    GVM_std title "STD = ".sprintf("%.3f", GVM_std)." kV",\
    GVM_avg_unce title "AVG_unc = ".sprintf("%.3f", GVM_avg_unce)." kV"


# Bottom Panel: Terminal Voltage (TV)
set xlabel "Time [h:m]"
set ylabel "TV [kV]"
set yrange [TV_ave - 20 : TV_ave + 20]
set format x "%H:%M"
plot TV u 1:2 t "TV [kV]" pt 6, \
     TV_ave title "Average TV [kV] = ".sprintf("%.2f",TV_ave)." kV"

unset multiplot
set output