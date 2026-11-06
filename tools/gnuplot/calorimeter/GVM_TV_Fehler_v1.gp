reset
reset session
set colorsequence podo

# set locale "German"
#set decimalsign locale ','
# set decimalsign ',' 
# set decimalsign ','
# set locale 'de_DE'
# set decimal locale 'de_DE.UTF-8'
set decimalsign ','
set datafile sep '\t'
set datafile columnheaders
set xdata time
set timefmt "%d.%m.%Y %H:%M:%S"

### Paths - To be changed every time
########################
# pathgas = "/net/files/share/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/GastargetLog/2606"
pathgas = "/net/files/share/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/PelletronLog/2504"

### Filenames - To be changed every time
########################
#LISTgas = system("ls ".pathgas."/Log_250511_*.csv")
LISTgas = system("ls ".pathgas."/Log_250423_*.csv ".pathgas."/Log_250424_*.csv")

### Date and time - To be changed every time
#######################
start = strptime("%d.%m.%Y %H:%M:%S", "23.04.2025 16:26:00")
stop  = strptime("%d.%m.%Y %H:%M:%S", "23.04.2025 16:36:00")
Run_number = 4368

### Temporary data files
temp = "temp.txt"
GVM_tv = "GVM-TV-Fehler.txt" 
TV = "TV.txt"

### Create output directory for the run
output_dir = sprintf("GVM-TV/Run%d", Run_number)
system(sprintf("mkdir -p %s", output_dir))

set table temp
    plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.7f",column(58))):(sprintf("%.7f",column(59))) with table
unset table

set print GVM_tv
    print "time GVM-TV Fehler [V]"
unset print
set xdata
set table GVM_tv
    plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3*1000)) with table if ($2>start&&$2<stop)
unset table

set print TV
    print "time TV[kV]"
unset print
set xdata
set table TV
    plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$4)) with table if ($2>start&&$2<stop)
unset table


### Get averages from the temporary files
set xdata
stats TV u 1:2 nooutput
TV_ave = STATS_mean_y
stats GVM_tv u 1:2 nooutput
GVM_avg = STATS_mean_y
# Calculate the true RMS using the mean and standard deviation
stats GVM_tv u 1:2 nooutput
GVM_rms = sqrt(STATS_mean_y**2 + STATS_stddev_y**2)
# Mean absolute error 
stats GVM_tv u 1:(abs($2)) skip 1 nooutput
GVM_mae = STATS_mean_y
### Calculate duration in seconds
duration_seconds = stop - start


### Setup plotting terminal for PNG generation
set xdata time
set timefmt "%s"
set terminal pngcairo enhanced size 1200,800
set output sprintf("%s/GVM-TV_Fehler_Run%d.png", output_dir, Run_number)

set multiplot layout 2,1

# Top Panel: GVM-TV Fehler
set ylabel "GVM-TV Fehler [V]"
set yrange [-2.0 : 2]
set format x ""
unset xlabel
# plot GVM_tv u 1:2 t "GVM-TV Fehler [V]" pt 6, \
#     GVM_rms title "RMS GVM-TV Fehler [V] = ".sprintf("%.5f",GVM_rms)." V"
plot GVM_tv u 1:2 t "GVM-TV Fehler [V]" pt 6, \
    GVM_rms title "RMS = ".sprintf("%.2f",GVM_rms)." V", \
    GVM_mae title "MAE = ".sprintf("%.2f",GVM_mae)." V", \
    GVM_avg title "AVG = ".sprintf("%.2f", GVM_avg)." V"

# Bottom Panel: Terminal Voltage (TV)
set xlabel "Time [h:m]"
set ylabel "TV [kV]"
set yrange [TV_ave - 20 : TV_ave + 20]
set format x "%H:%M"
plot TV u 1:2 t "TV [kV]" pt 6, \
    TV_ave title "Average TV [kV] = ".sprintf("%.4f",TV_ave)." kV"

unset multiplot
set output