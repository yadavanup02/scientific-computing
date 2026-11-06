import pandas as pd
import numpy as np
from scipy.interpolate import interp1d

# --- 1. Load Data ---
# Load Iliadis 2010 rate file
iliadis = pd.read_csv("RatesIliadis_final/RatesMC.out", delim_whitespace=True, comment="#", header=None)
T9_il = iliadis.iloc[:, 0].values
rate_low_il = iliadis.iloc[:, 1].values
rate_med_il = iliadis.iloc[:, 2].values
rate_hi_il  = iliadis.iloc[:, 3].values

# Load This Work rate file
this = pd.read_csv("RatesAnup_final/RatesMC.out", delim_whitespace=True, comment="#", header=None)
T9_th = this.iloc[:, 0].values
rate_low_th = this.iloc[:, 1].values
rate_med_th = this.iloc[:, 2].values
rate_hi_th  = this.iloc[:, 3].values

# Load NACRE data from Gorres200.txt
Gor = pd.read_csv("Gorres200.txt", delim_whitespace=True, comment="#", header=0)
T9_nacre = Gor.iloc[:, 0].values
rate_med_nacre = Gor.iloc[:, 5].values
rate_low_nacre = Gor.iloc[:, 6].values
rate_hi_nacre = Gor.iloc[:, 7].values

# --- 2. Calculate Fractional Uncertainties (Percentage) ---
# Formula: ((High - Low) / Median) * 100
uncert_il = ((rate_hi_il - rate_low_il) / rate_med_il) * 100
uncert_th = ((rate_hi_th - rate_low_th) / rate_med_th) * 100
uncert_nacre = ((rate_hi_nacre - rate_low_nacre) / rate_med_nacre) * 100

# --- 3. Setup Interpolation ---
# Interpolate uncertainties
f_uncert_il = interp1d(T9_il, uncert_il, kind='linear', bounds_error=False, fill_value="extrapolate")
f_uncert_th = interp1d(T9_th, uncert_th, kind='linear', bounds_error=False, fill_value="extrapolate")
f_uncert_nacre = interp1d(T9_nacre, uncert_nacre, kind='linear', bounds_error=False, fill_value="extrapolate")

# Interpolate median rates (This fixes the array-calling error)
f_rate_il = interp1d(T9_il, rate_med_il, kind='linear', bounds_error=False, fill_value="extrapolate")
f_rate_th = interp1d(T9_th, rate_med_th, kind='linear', bounds_error=False, fill_value="extrapolate")
f_rate_nacre = interp1d(T9_nacre, rate_med_nacre, kind='linear', bounds_error=False, fill_value="extrapolate")

# --- 4. Query Specific Astrophysical Temperatures ---
# target_T9_values = [
#     0.070, 0.080, 0.090, 0.100, 0.150, 0.200, 0.300, 0.400, 
#     0.500, 0.600, 0.700, 0.800, 0.900, 1.000, 1.500, 2.000, 
#     3.000, 4.000, 5.000, 6.000, 7.000, 8.000, 9.000, 10.000
# ]
target_T9_values = [
0.010,
0.011,
0.012,
0.013,
0.014,
0.015,
0.016,
0.018,
0.020,
0.025,
0.030,
0.040,
0.050,
0.060,
0.070,
0.080,
0.090,
0.100,
0.110,
0.120,
0.130,
0.140,
0.150,
0.160,
0.180,
0.200,
0.250,
0.300,
0.350,
0.400,
0.450,
0.500,
0.600,
0.700,
0.800,
0.900,
1.000,
1.250,
1.500,
1.750,
2.000,
2.500,
3.000,
3.500,
4.000,
5.000,
6.000,
7.000,
8.000,
9.000,
10.000
]

output_filename = "Rates_Comparison_Table_Iliadis.txt"

# Print formatting for the console
print("\n=== Reaction Rate Ratios & Uncertainty Reductions ===")
print(f"{'T9 (GK)':<8} | {'Ratio (Il/This)':<15} | {'Ratio (NACRE/This)':<18} | {'Uncert Red. (Il)':<16} | {'Uncert Red. (NACRE)'}")
print("-" * 85)

# Open a new text file in write mode ('w')
with open(output_filename, 'w') as f:
    
    # Write the tab-separated header line
    f.write("T9 (GK)\tRatio (Iliadis/This)\tRatio (NACRE/This)\tRed. Factor (Iliadis)\tRed. Factor (NACRE)\n")
    
    for t in target_T9_values:
        # 1. Evaluate interpolated uncertainties
        u_il = float(f_uncert_il(t))
        u_nacre = float(f_uncert_nacre(t))
        u_th = float(f_uncert_th(t))
        
        # 2. Evaluate interpolated median rates
        r_il = float(f_rate_il(t))
        r_nacre = float(f_rate_nacre(t))
        r_th = float(f_rate_th(t))
        
        # 3. Calculate factors and ratios (protecting against division by zero just in case)
        red_factor_il = u_il / u_th if u_th != 0 else 0
        red_factor_nacre = u_nacre / u_th if u_th != 0 else 0
        
        ratio_il = r_il / r_th if r_th != 0 else 0
        ratio_nacre = r_nacre / r_th if r_th != 0 else 0
        
        # 4. Write to file (tab-separated, using scientific notation for the ratios since they can vary widely)
        f.write(f"{t:.3f}\t{ratio_il:.1f}\t{ratio_nacre:.1f}\t{red_factor_il:.2f}\t{red_factor_nacre:.2f}\n")
        
        # 5. Print to console so you can see it running
        print(f"{t:<8.3f} | {ratio_il:<15.4e} | {ratio_nacre:<18.4e} | {red_factor_il:<16.2f} | {red_factor_nacre:<16.2f}")

print("-" * 85)
print(f"Success: Comparison table saved to '{output_filename}'")