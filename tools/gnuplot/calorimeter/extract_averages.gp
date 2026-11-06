reset
reset session
set colorsequence podo
set datafile sep '\t'
set datafile columnheaders
set xdata time
set timefmt "%d.%m.%Y %H:%M:%S"
set decimal locale 'de_DE.UTF-8'

### Paths - To be changed every time
		########################
pathpel = "~/share/Felsenkeller/Notfallinformationen/Pelletron/Logs/PelletronLog/2502"
pathgas = "~/share/Felsenkeller/Notfallinformationen/Pelletron/Logs/GastargetLog/2502"

### Filenames - To be changed every time
		########################
LISTpel = system("ls ".pathpel."/Log_250217_*.csv")
LISTgas = system("ls ".pathgas."/Log_250217_*.csv")

### Date and time - To be changed every time
			########################
start = strptime("%d.%m.%Y %H:%M:%S", "17.02.2025 14:12:00,0")
stop  = strptime("%d.%m.%Y %H:%M:%S", "17.02.2025 14:18:00,0")

### Temporary data files
temp = "temp.txt"
tacu = "target_current.txt"
capo = "calorimeter_power.txt"

### Read current from logfile and write into temporary file
set table temp
	plot for [data in LISTpel] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.7f",column(86))) with table
unset table
set print tacu
	print "time	current [uA]"
unset print
set xdata
set table tacu
	plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)) with table if ($2>start&&$2<stop)
unset table

### Read power from logfile and write into temporary file
set table temp
	plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.7f",column(36))) with table
unset table
set print capo
	print "time	heater power [W]"
unset print
set xdata
set table capo
	plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)) with table if ($2>start&&$2<stop)
unset table

### Get averages from the temporary files
set xdata
stats tacu u 1:2 nooutput
tacu_ave = STATS_mean_y
stats capo u 1:2 nooutput
capo_ave = STATS_mean_y

set xdata time
set timefmt "%s"
set format x '%H:%M'
set multiplot layout 2,1
set key outside center top
plot	capo u 1:2 t "calorimeter power [W]" pt 6,\
		capo_ave t "average colorimeter power = ".gprintf("%.3f",capo_ave)." W"
plot	tacu u 1:2 t "target current [uA]" pt 6,\
		tacu_ave t "average target current = ".gprintf("%.3f",tacu_ave)." uA"
unset multiplot
