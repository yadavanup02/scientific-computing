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
pathgas = "/net/files/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/GastargetLog/2504"

### Filenames - To be changed every time
        ########################
#LISTgas = system("ls ".pathgas."/Log_250511_*.csv")
LISTgas = system("ls ".pathgas."/Log_250428_*.csv ".pathgas."/Log_250429_*.csv")

### Date and time - To be changed every time
        ########################4443	1150.05	28/04/2025 18:52	29/04/2025 08:53	50465
start = strptime("%d.%m.%Y %H:%M:%S", "28.04.2025 18:52:00")
stop  = strptime("%d.%m.%Y %H:%M:%S", "29.04.2025 08:52:00")
Run_number = 4443
zero_power  = 97.01 # W
beam_energy = 1150.05  # keV
energy_loss = 43.02 # keV

### Temporary data files
temp = "temp.txt"
capo = "calorimeter_power.txt" 
tacu = "target_current.txt"

### Create output directory for the run
output_dir = sprintf("Calorimeter_Power/Run%d", Run_number)
system(sprintf("mkdir -p %s", output_dir))

set table temp
    plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.3f",column(48))):((zero_power - column(48))*1000 / (beam_energy - energy_loss)) with table
unset table

set print capo
	print "time	heater power [W]"
unset print
set xdata
set table capo
	plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)) with table if ($2>start&&$2<stop)
unset table

set print tacu
    print "time	current [uA]"
unset print
set xdata
set table tacu
    plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$4)) with table if ($2>start&&$2<stop)
unset table


### Get averages from the temporary files
set xdata
stats tacu u 1:2 nooutput
tacu_ave = STATS_mean_y
stats capo u 1:2 nooutput
capo_ave = STATS_mean_y
### Calculate duration in seconds
duration_seconds = stop - start

### Save plots: Power and Beam Current
set xdata time
set timefmt "%s"
set format x '%H:%M'
#set title sprintf("Run %d: Power and Beam Current (Duration: %d seconds)", Run_number, duration_seconds)
#set key outside center top
set terminal postscript landscape
set output sprintf("%s/beam_current_Run%d.ps", output_dir, Run_number)
#set output sprintf("%s/%s", output_dir, capo)

set multiplot layout 2,1 

# Plot calorimeter power
set ylabel "Calorimeter power [W]"
#set yrange [capo_ave -5 : capo_ave + 5]
set format x ""
unset xlabel
plot capo u 1:2 t "Calorimeter power [W]" pt 6,\
    capo_ave t "Average calorimeter power = ".gprintf("%.3f",capo_ave)." W"

# Plot beam current
set xlabel "Time [h:m]"
set ylabel "Beam current [uA]"
#set yrange [tacu_ave - 10 : tacu_ave + 10]
set format x "%H:%M"
plot tacu u 1:2 t "Beam current [uA]" pt 6,\
    tacu_ave t "Average beam current = ".gprintf("%.3f",tacu_ave)." uA"

unset multiplot
set output
system "ps2pdf ".sprintf("%s/Power_Current_Run%d.ps", output_dir, Run_number)." ".sprintf("%s/Power_current_Run%d_v3.pdf", output_dir, Run_number)
unset output
print sprintf("Calorimeter power average: %.3f W", capo_ave)
print sprintf("Beam current average: %.3f uA", tacu_ave)

# Display the plot on screen
set terminal qt
set multiplot layout 2,1 

# Plot calorimeter power
set ylabel "Calorimeter power [W]"
set yrange [capo_ave -5 : capo_ave + 5]
set format x ""
unset xlabel
plot capo u 1:2 t "Calorimeter power [W]" pt 6,\
    capo_ave t "Average calorimeter power = ".gprintf("%.3f",capo_ave)." W"

# Plot beam current
set xlabel "Time [h:m]"
set ylabel "Beam current [uA]"
set yrange [tacu_ave - 5 : tacu_ave + 5]
set format x "%H:%M"
plot tacu u 1:2 t "Beam current [uA]" pt 6,\
    tacu_ave t "Average beam current = ".gprintf("%.3f",tacu_ave)." uA"

unset multiplot
