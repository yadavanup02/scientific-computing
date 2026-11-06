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
pathgas = "~/share/Felsenkeller/Notfallinformationen/Pelletron/Logs/GastargetLog/2505"

### Filenames - To be changed every time
        ########################
#LISTgas = system("ls ".pathgas."/Log_250511_*.csv")
LISTgas = system("ls ".pathgas."/Log_250530_*.csv ".pathgas."/Log_250531_*.csv")

### Date and time - To be changed every time
        ########################
start = strptime("%d.%m.%Y %H:%M:%S", "30.05.2025 12:30:00")
stop  = strptime("%d.%m.%Y %H:%M:%S", "31.05.2025 08:00:00")
Run_number = 4723

### Temporary data files
temp = "temp.txt"
mfcf = "mfc_flow.txt" 
mfcp = "mfc_pressure.txt"

### Create output directory for the run
output_dir = sprintf("Calorimeter_Power/Run%d", Run_number)
system(sprintf("mkdir -p %s", output_dir))

set table temp
    plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.3f",column(50))):(sprintf("%.3f",column(52))) with table
unset table

set print mfcf
	print "Flow rate [ln/min]"
unset print
set xdata
set table mfcf
	plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)) with table if ($2>start&&$2<stop)
unset table

set print mfcp
    print "OInlet pressure [bar]"
unset print
set xdata
set table mfcp
    plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$4)) with table if ($2>start&&$2<stop)
unset table

### Get averages from the temporary files
set xrange [*:*]
set xdata
stats mfcf u 1:2 nooutput
mfcf_ave = STATS_mean_y
stats mfcp u 1:2 nooutput
mfcp_ave = STATS_mean_y
### Calculate duration in seconds
duration_seconds = stop - start

### Save plots: Power and Beam Current
set xdata time
set timefmt "%s"
set format x '%H:%M'

# Display the plot on screen
set terminal qt
set multiplot layout 2,1 

# Plot MFC flow rate
set ylabel "Flow rate [ln/min]"
set yrange [mfcf_ave -5 : mfcf_ave + 5]
set format x ""
unset xlabel
plot mfcf u 1:2 t "MFC flow rate [ln/min]" pt 6,\
    mfcf_ave t "Average MFC flow rate = ".gprintf("%.3f",mfcf_ave)." ln/min"

# Plot MFC pressure
set xlabel "Time [h:m]"
set ylabel "MFC pressure [bar]"
set yrange [mfcp_ave - 5 : mfcp_ave + 5]
set format x "%H:%M"
plot mfcp u 1:2 t "MFC pressure [bar]" pt 6,\
    mfcp_ave t "Average MFC pressure = ".gprintf("%.3f",mfcp_ave)." bar"

unset multiplot
