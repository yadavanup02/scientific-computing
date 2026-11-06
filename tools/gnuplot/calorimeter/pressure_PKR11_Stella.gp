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
pathgas = "/net/files/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/GastargetLog/2510"

### Filenames - To be changed every time
        ########################
#LISTgas = system("lso ".pathgas."/Log_250511_*.csv")
LISTgas = system("ls ".pathgas."/Log_251020_*.csv ".pathgas."/Log_251022_*.csv")

### Date and time - To be changed every time
        ########################
start = strptime("%d.%m.%Y %H:%M:%S", "20.10.2025 15:00:00")
stop  = strptime("%d.%m.%Y %H:%M:%S", "22.10.2025 09:08:00")

### Temporary data files
temp = "temp.txt"
capo = "pressure_pkr11.txt" 
tacu = "target_current.txt"
date = 251014

### Create output directory for the run
output_dir = sprintf("STELLA_pressure/Date%d", date)
system(sprintf("mkdir -p %s", output_dir))

set table temp
    plot for [data in LISTgas] data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.9f",column(9))) with table
unset table

set print capo
	print "time	PKR11 Pressure [mbar]"
unset print
set xdata
set table capo
	plot temp u (sprintf("%.1f",$2)):(sprintf("%.9f",$3)) with table if ($2>start&&$2<stop)
unset table
duration_seconds = stop - start

### Save plots: Power and Beam Current
set xdata time
set timefmt "%s"
set format x '%H:%M'
set ylabel "pressure [mbar]"
set format y "%.7f"

#set key outside center top
set terminal postscript landscape
set output sprintf("%s/PKR11_Pressure%d.ps", output_dir, date)
#set format x ""
#unset xlabel
plot capo u 1:2 with lines title "PKR11 Pressure" linecolor rgb "blue" lw 2
    #pkr1_ave t "Average PKR11 pressure = ".gprintf("%.7f",pkr1_ave)." mbar"
set output
system "ps2pdf ".sprintf("%s/PKR11_Pressure%d.ps", output_dir, date)." ".sprintf("%s/PKR11_Pressure%d_v3.pdf", output_dir, date)
unset output
set terminal qt
# set multiplot layout 2,1 

# Plot calorimeter power
set ylabel "Pressure [mbar]"
set xlabel "Time [hh:mm]"
#set yrange [capo_ave -5 : capo_ave + 5]
#set format x ""
set ytics format "10^{%L}"
set logscale y
set ytics
#set y2tics
set mytics 9

do for [n=-7:3] {
  set ytics add (sprintf("10^{%d}", n) 10**n)
  do for [m=11:99] {
  value = m/10.0 * 10**n 
  set ytics add (sprintf("%.1f*10^{%d}", m/10.0, n) value)
  
  set arrow from graph 0, first value to graph 1, first value nohead lc rgb "gray" lw 0.5
                 }
                }

set grid y front
set yrange [5e-7:9e-4]
#set format y "%.9g"
unset xlabel
plot capo u 1:2  with lines title "PKR11 Pressure" linecolor rgb "blue" lw 2
    #temp u 1:3 t "pressure [mbar]" pt 7
    #mfcf_ave t "Average MFC flow rate = ".gprintf("%.3f",mfcf_ave)." mbar"
