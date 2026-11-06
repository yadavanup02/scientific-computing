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
LISTgas = system("ls ".pathgas."/Log_251014_*.csv")

### Date and time - To be changed every time
        ########################
start = strptime("%d.%m.%Y %H:%M:%S", "14.10.2025 06:21:00")
stop  = strptime("%d.%m.%Y %H:%M:%S", "14.10.2025 09:35:00")
Run_number = 4429
zero_power  = 96.420
beam_energy = 1169.770 # MeV
energy_loss = 36.060 # MeV

### Temporary data files
temp = "temp.txt"

### Create output directory for the run
output_dir = sprintf("Calorimeter_Power/Run%d", Run_number)
system(sprintf("mkdir -p %s", output_dir))

### Update combined file path to save in the run directory
combined_file = sprintf("%s/combined_data.txt", output_dir)

### Read power from logfile and write into a single file with time, power, and current
set table temp
    plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.3f",column(8))):((zero_power - column(8))*1000 / (beam_energy - energy_loss)) with table
unset table

# Convert commas to periods in temp file
system("sed -i 's/,/./g' ".temp)

set print combined_file
    print "time\tpower [W]\tcurrent [uA]"
set table combined_file
    plot temp u (strptime("%s", stringcolumn(1))):2:3 with table if (strptime("%s", stringcolumn(1)) > start && (strptime("%s", stringcolumn(1)) < stop))
unset table
unset print

### Get averages from the combined file
set xdata
#stats combined_file u 2 nooutput
#capo_ave = STATS_mean_y
#stats combined_file u 3 nooutput
#current_ave = STATS_mean_y

### Calculate duration in seconds
duration_seconds = stop - start

### Save plots: Power and Beam Current
set xdata time
set xlabel "Time [hh:mm]"
set timefmt "%s"
set format x '%H:%M'
set title sprintf("Run %d: Power and Beam Current (Duration: %d seconds)", Run_number, duration_seconds)
set key outside center top
set terminal svg size 800,600
set output sprintf("%s/Power_and_Current_Run%d.svg", output_dir, Run_number)

set multiplot layout 2,1 title sprintf("Run %d: Power and Beam Current", Run_number)

# Plot calorimeter power
set ylabel "Calorimeter power [W]"
#set yrange [capo_ave - 3 : capo_ave + 3]
plot combined_file u 1:2 t "Calorimeter power [W]" pt 6
    #capo_ave t "Average calorimeter power = ".gprintf("%.3f",capo_ave)." W"

# Plot beam current
#set ylabel "Beam current [uA]"
#plot combined_file u 1:3 t "Beam current [uA]" pt 6,\
#    current_ave t "Average beam current = ".gprintf("%.3f",current_ave)." uA"

#unset multiplot
#unset output

# Display the plot on screen
set terminal qt
replot