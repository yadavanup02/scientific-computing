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
#pathpel = "~/share/Felsenkeller/Notfallinformationen/Pelletron/Logs/PelletronLog/2504"
pathgas = "~/share/Felsenkeller/Notfallinformationen/Pelletron/Logs/GastargetLog/2504"

### Filenames - To be changed every time
    #########################
#LISTgas = system("ls ".pathgas."/Log_250426_*.csv ".pathgas."/Log_250427_*.csv")
LISTgas = system("ls ".pathgas."/Log_250427_*.csv")
### Date and time - To be changed every time
        ########################
start = strptime("%d.%m.%Y %H:%M:%S", "27.04.2025 14:21:00")
stop  = strptime("%d.%m.%Y %H:%M:%S", "27.04.2025 14:35:00")
Run_number = 4429
zero_power  = 94.0
beam_energy = 1.136 # MeV
energy_loss = 0.030 # MeV


### Temporary data files
temp = "temp.txt"
tacu = "target_current.txt"

### Create output directory for the run

output_dir = sprintf("Calorimeter_Power/Run%d", Run_number)
system(sprintf("mkdir -p %s", output_dir))

### Update calorimeter power file path to save in the run directory
capo = sprintf("%s/calorimeter_power.txt", output_dir)

### Read current from logfile and write into temporary file
# set table temp
#     plot for [data in LISTpel] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.7f",column(86))) with table
# unset table
# set print tacu
#     print "time	current [uA]"
# unset print
# set xdata
# set table tacu
#     plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)) with table if ($2>start&&$2<stop)
# unset table

### Read power from logfile and write into temporary file
set table temp
    plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.7f",column(48))) with table
unset table
set print capo
    print "time	heater power [W]"
unset print
set xdata
set table capo
    plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)) with table if ($2>start&&$2<stop)
unset table
# beam_current =  (zero_power - capo) / (beam_energy - energy_loss) # uA

### Get averages from the temporary files
set xdata
stats tacu u 1:2 nooutput
tacu_ave = STATS_mean_y
stats capo u 1:2 nooutput
capo_ave = STATS_mean_y


### Calculate duration in seconds
duration_seconds = stop - start

### Save only the power plot
set xdata time
set xlabel "Time [h:m]"
set ylabel "Calorimeter power [W]"
set timefmt "%s"
set format x '%H:%M'
set yrange [capo_ave - 5 : capo_ave + 5]
set title sprintf("Run time (Duration: %d seconds)", duration_seconds)
set key outside center top
set terminal svg size 800,600
set output sprintf("%s/Power_Plot_Run%d.svg", output_dir, Run_number)
plot capo u 1:2 t "calorimeter power [W]" pt 6,\
    capo_ave t "average calorimeter power = ".gprintf("%.3f",capo_ave)." W"

# Display the plot on screen
print sprintf("Calorimeter power average: %.3f W", capo_ave)
set terminal qt
unset output
replot
unset output