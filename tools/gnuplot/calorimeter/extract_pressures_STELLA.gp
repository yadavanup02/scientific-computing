reset
reset session
set colorsequence podo
set datafile sep '\t'
set datafile columnheaders
set xdata time
set timefmt "%d.%m.%Y %H:%M:%S"
set decimal locale 'de_DE.UTF-8'
#set decimalsign locale

### Paths - To be changed every time
        ########################
pathgas = "~/share/Felsenkeller/Notfallinformationen/Pelletron/Logs/GastargetLog/2510"

### Filenames - To be changed every time
        ########################
#LISTgas = system("ls ".pathgas."/Log_250511_*.csv")
LISTgas = system("ls ".pathgas."/Log_251014_*.csv ".pathgas."/Log_251014_*.csv")

### Date and time - To be changed every time
        ########################
start = strptime("%d.%m.%Y %H:%M:%S", "14.10.2025 06:00:00")
stop  = strptime("%d.%m.%Y %H:%M:%S", "14.10.2025 08:43:00")
Run_number = 4723

### Temporary data files
temp = "temp.txt"
mfcf = "mfc_flow.txt" 
mfcp = "mfc_pressure.txt"

### Create output directory for the run
output_dir = sprintf("Calorimeter_Power/Run%d", Run_number)
system(sprintf("mkdir -p %s", output_dir))

set table $TEMP
    plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.3f",$8)):(sprintf("%.3f",$9)) with table
unset table

set print temp
	print "# PKR-11_ PKR-12_"
	print "# [mbar] [mbar]"
unset print
set xdata
set table temp
	plot $TEMP u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)):(sprintf("%.7f",$4)) with table if ($2>start&&$2<stop)
unset table

### Get averages from the temporary files
set xrange [*:*]
set xdata
stats temp u 1:2 nooutput
pkr1_ave = STATS_mean_y
stats temp u 1:3 nooutput
pkr2_ave = STATS_mean_y
### Calculate duration in seconds
duration_seconds = stop - start

### Save plots: Power and Beam Current
set xdata time
set timefmt "%s"
set format x '%H:%M'

# Display the plot on screen
set terminal qt
set ylabel "pressure [mbar]"
#set yrange [mfcf_ave -5 : mfcf_ave + 5]
set format x ""
unset xlabel
plot temp u 1:2 t "pressure [mbar]" pt 6
     #temp u 1:3 t "pressure [mbar]" pt 7
    #mfcf_ave t "Average MFC flow rate = ".gprintf("%.3f",mfcf_ave)." mbar"

# Plot MFC pressure
set xlabel "Time [h:m]"
set ylabel "pressure PKR11 [mbar]"
#set yrange [mfcp_ave - 5 : mfcp_ave + 5]
set format x "%H:%M"
plot temp u 1:2 t "pressure [mbar]" pt 6
   # mfcp_ave t "Average MFC pressure = ".gprintf("%.3f",mfcp_ave)." bar"

unset multiplot
