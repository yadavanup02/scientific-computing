reset
reset session
set colorsequence podo

### Master paths and log selection
# pathgas = "/net/files/share/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/PelletronLog/2504"
# LISTgas = system("ls ".pathgas."/Log_250426_*.csv ".pathgas."/Log_250427_*.csv ".pathgas."/Log_250428_*.csv ".pathgas."/Log_250429_*.csv")
### Define base paths for both April (2504) and May (2505)
path_apr = "/net/files/share/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/PelletronLog/2504"
path_may = "/net/files/share/fwk/fwkk/Felsenkeller/Notfallinformationen/Pelletron/Logs/PelletronLog/2505"

### Use Linux wildcards to grab ranges without writing every single day
# 1. "Log_250429_*.csv" and "Log_250430_*.csv" handles late April
# 2. "Log_25050*_*.csv" grabs May 01 through May 09
# 3. "Log_25051[0-4]_*.csv" grabs exactly May 10 through May 14
# LISTgas = system("ls ".path_apr."/Log_250429_*.csv ".path_apr."/Log_250430_*.csv ".path_may."/Log_25050*_*.csv ".path_may."/Log_25051[0-4]_*.csv")
# LISTgas = system("ls ".path_apr."/Log_250426_*.csv ".path_apr."/Log_250427_*.csv ".path_apr."/Log_250428_*.csv ".path_apr."/Log_250429_*.csv ")
LISTgas = system("ls ".path_apr."/Log_250423_*.csv ".path_apr."/Log_250424_*.csv ".path_apr."/Log_250425_*.csv")
# Point this to your new text file
CONFIG_FILE = 'runs_1618.txt'
MASTER_OUT  = 'runs_1618_ouput.txt'

# Initialize the master summary file with headers
set print MASTER_OUT
print "Run_Number\tGVM_std_kV\tGVM_rms_kV\tGVM_avg_kV\tGVM_avg_unc"
unset print

### STEP 1: Parse the configuration file to count the rows
set xdata  # Ensure time mode is off so stats can count lines safely
stats CONFIG_FILE skip 1 nooutput
DATA_ROWS = STATS_records

### STEP 2: Loop over every run configuration row
do for [i=0:DATA_ROWS-1] {
    
    # --- THE CRITICAL FIX ---
    # Clear persistent plot ranges from the previous loop so stats doesn't filter out valid data!
    unset xrange
    unset yrange
    set autoscale
    
    # Calculate line number (skip header)
    line_num = i + 2
    
    # Use awk to robustly extract columns regardless of spaces or tabs
    Run_number = int(system(sprintf("sed -n '%dp' %s | awk '{print $1}'", line_num, CONFIG_FILE)))
    start_str  = system(sprintf("sed -n '%dp' %s | awk '{print $2, $3}'", line_num, CONFIG_FILE))
    stop_str   = system(sprintf("sed -n '%dp' %s | awk '{print $4, $5}'", line_num, CONFIG_FILE))
    
    # Convert string bounds. (Parses the colons in your runs_list.txt dates: 24:04:2025)
    start = strptime("%d:%m:%Y %H:%M:%S", start_str)
    stop  = strptime("%d:%m:%Y %H:%M:%S", stop_str)
    duration_seconds = stop - start

    ### Temporary data files
    temp = "temp.txt"
    GVM_tv = "GVM-TV-Fehler.txt" 
    TV = "TV.txt"

    ### Create output directory for the run
    output_dir = sprintf("GVM-TV/Run%d", Run_number)
    system(sprintf("mkdir -p %s", output_dir))

    # --- Data Extraction Phase ---
    set datafile sep '\t'
    set datafile columnheaders
    set xdata time
    
    # Switch parser to match the raw CSV logs (dots)
    set timefmt "%d.%m.%Y %H:%M:%S" 
    
    set table temp
        plot for [data in LISTgas] "< sed 's/,/./g' ".data u (a=timecolumn(1)):(sprintf("%.1f",a)):(sprintf("%.7f",column(58))):(sprintf("%.7f",column(59))) with table
    unset table

    # --- Data Filtering Phase ---
    # Turn OFF xdata so we can cleanly evaluate $2 as a number against 'start' and 'stop'
    set xdata
    
    set print GVM_tv
        print "time GVM-TV Fehler [kV]"
    unset print
    set table GVM_tv
        plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$3)) with table if ($2>start&&$2<stop)
    unset table

    set print TV
        print "time TV[kV]"
    unset print
    set table TV
        plot temp u (sprintf("%.1f",$2)):(sprintf("%.7f",$4)) with table if ($2>start&&$2<stop)
    unset table


    # # --- Calculate Statistics & CHECK FOR EMPTY DATA ---
    # stats TV u 1:2 skip 1 nooutput
    
    # # SAFETY CHECK: Did we find any points? 
    # if (STATS_records == 0) {
    #     print "\n============================================="
    #     print sprintf("  Run Number:          %d", Run_number)
    #     print "  STATUS:              NO DATA FOUND - SKIPPED"
    #     print "=============================================\n"
    #     continue  # <--- This tells the loop to immediately jump to the next run
    # }
    # TV_ave = STATS_mean_y

    # stats GVM_tv u 1:2 skip 1 nooutput
    # if (STATS_records == 0) {
    #     print "\n============================================="
    #     print sprintf("  Run Number:          %d", Run_number)
    #     print "  STATUS:              NO GVM DATA - SKIPPED"
    #     print "=============================================\n"
    #     continue
    # }
    # GVM_avg = STATS_mean_y
    # GVM_std = STATS_stddev_y
    # GVM_rms = sqrt(STATS_mean_y**2 + STATS_stddev_y**2)

    # stats GVM_tv u 1:(abs($2)) skip 1 nooutput
    # if (STATS_records == 0) {
    #     print "\n============================================="
    #     print sprintf("  Run Number:          %d", Run_number)
    #     print "  STATUS:              NO GVM DATA - SKIPPED"
    #     print "=============================================\n"
    #     continue
    # }
    # GVM_mae = STATS_mean_y
    # --- SAFETY CHECK BEFORE STATS ---
    # Use Linux to count the lines in the generated files. 
    # Since we print a header row, an empty data file will have exactly 1 line.
    lines_TV = int(system("wc -l < TV.txt"))
    lines_GVM = int(system("wc -l < GVM-TV-Fehler.txt"))

    if (lines_TV <= 1 || lines_GVM <= 1) {
        print "\n============================================="
        print sprintf("  Run Number:          %d", Run_number)
        print "  STATUS:              NO DATA FOUND - SKIPPED"
        print "=============================================\n"
        continue  # Instantly skips to the next run without breaking the stats engine!
    }

    # --- Calculate Statistics ---
    # We only reach this point if we mathematically proved there is data in the files
    stats TV u 1:2 skip 1 nooutput
    TV_ave = STATS_mean_y

    stats GVM_tv u 1:2 skip 1 nooutput
    GVM_avg = STATS_mean_y
    GVM_avg_unc  = STATS_stddev_y / sqrt(STATS_records)

    GVM_std = STATS_stddev_y
    GVM_rms = sqrt(STATS_mean_y**2 + STATS_stddev_y**2)

    stats GVM_tv u 1:(abs($2)) skip 1 nooutput
    GVM_mae = STATS_mean_y

    # --- Append to Master Summary File ---
    set print MASTER_OUT append
    print sprintf("%d\t%.2f\t%.2f\t%.2f\t%.4f", Run_number, GVM_std, GVM_rms, GVM_avg, GVM_avg_unc)
    unset print

    # --- Console Output ---
    print "\n============================================="
    print sprintf("  Run Number:          %d", Run_number)
    print sprintf("  Duration:            %d seconds", duration_seconds)
    print "============================================="
    print sprintf("  GVM-TV Std Dev (σ):  %.5f kV", GVM_std)
    print sprintf("  GVM-TV RMS:          %.5f kV", GVM_rms)
    print sprintf("  GVM-TV AVG_unc:      %.5f kV", GVM_avg_unc)
    print "=============================================\n"

    # --- Plot Rendering ---
    # Turn time mode back on for plotting the final graphics
    set xdata time
    set timefmt "%s"
    set terminal pngcairo enhanced size 1200,800
    set output sprintf("%s/GVM-TV-Fehler_Run%d.png", output_dir, Run_number)

    set multiplot layout 2,1

    # Top Panel
    set ylabel "GVM-TV Fehler [kV]"
    set yrange [GVM_rms-1 : GVM_rms + 1]
    set format x ""
    unset xlabel
    plot GVM_tv u 1:2 t "GVM-TV Fehler [kV]" pt 6, \
         GVM_rms title "RMS = ".sprintf("%.5f",GVM_rms)." kV", \
         GVM_mae title "MAE = ".sprintf("%.5f",GVM_mae)." kV", \
         GVM_avg title "AVG = ".sprintf("%.5f", GVM_avg)." kV", \
         GVM_std title "STD = ".sprintf("%.5f", GVM_std)." kV",\
         GVM_avg_unc title "AVG_unc = ".sprintf("%.5f", GVM_avg_unc)." kV"


    # Bottom Panel
    set xlabel "Time [h:m]"
    set ylabel "TV [kV]"
    set yrange [TV_ave - 20 : TV_ave + 20]
    set format x "%H:%M"
    plot TV u 1:2 t "TV [kV]" pt 6, \
         TV_ave title "Average TV [kV] = ".sprintf("%.2f",TV_ave)." kV"

    unset multiplot
    set output
}