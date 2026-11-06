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
T9_NACRE = Gor.iloc[:, 0].values
rate_med_NACRE = Gor.iloc[:, 5].values
rate_low_NACRE = Gor.iloc[:, 6].values
rate_hi_NACRE = Gor.iloc[:, 7].values

# --- 2. Calculate Fractional Uncertainties (Percentage) ---
# Here we define the total uncertainty band width relative to the median rate.
# Formula: ((High - Low) / Median) * 100
# uncert_percent_il = ((rate_hi_il - rate_low_il) / rate_med_il) * 100
# uncert_percent_th = ((rate_hi_th - rate_low_th) / rate_med_th) * 100

# NACRE
uncert_percent_NACRE = ((rate_hi_NACRE - rate_low_NACRE) / rate_med_NACRE) * 100
uncert_percent_th = ((rate_hi_th - rate_low_th) / rate_med_th) * 100

# --- 3. Setup Interpolation ---
# This allows you to check the exact uncertainty at specific Gamow temperatures 
# without worrying if that exact T9 value exists in the array.
# f_uncert_il = interp1d(T9_il, uncert_percent_il, kind='linear', bounds_error=False, fill_value="extrapolate")
# f_uncert_th = interp1d(T9_th, uncert_percent_th, kind='linear', bounds_error=False, fill_value="extrapolate")

# NACRE
f_uncert_NACRE = interp1d(T9_NACRE, uncert_percent_NACRE, kind='linear', bounds_error=False, fill_value="extrapolate")
f_uncert_th = interp1d(T9_th, uncert_percent_th, kind='linear', bounds_error=False, fill_value="extrapolate")

# --- 4. Query Specific Astrophysical Temperatures ---
# Enter the T9 values (in GK) that are most relevant to your PRL discussion
# target_T9_values = [0.05, 0.08, 0.1, 0.2, 0.4, 0.5, 1.0, 2.0]
target_T9_values = [0.01,
0.02,
0.03,
0.04,
0.05,
0.06,
0.07,
0.08,
0.09,
0.10,
0.15,
0.20,
0.30,
0.40,
0.50,
0.60,
0.70,
0.80,
0.90,
1.00,
1.50,
2.00,
2.50,
3.00,
4.00,
5.00,
6.00,
7.00,
8.00,
9.00,
10.0
]
# target_T9_values = [0.070,
# 0.080,
# 0.090,
# 0.100,
# 0.150,
# 0.200,
# 0.300,
# 0.400,
# 0.500,
# 0.600,
# 0.700,
# 0.800,
# 0.900,
# 1.000,
# 1.500,
# 2.000,
# 3.000,
# 4.000,
# 5.000,
# 6.000,
# 7.000,
# 8.000,
# 9.000,
# 10.000]

print("\n=== Reaction Rate Uncertainty Reduction ===")
print(f"{'T9 (GK)':<10} | {'Iliadis Uncert (%)':<20} | {'This Work Uncert (%)':<22} | {'Reduction Factor'}")
print("-" * 75)
output_filename = "Uncertainty_Reduction_Table_NACRE.txt"

# Open a new text file in write mode ('w')
with open(output_filename, 'w') as f:
    
    # Write the tab-separated header line (\n adds the line break at the end)
    f.write("T9 (GK)\tNACRE Uncert (%)\tThis Work Uncert (%)\tReduction Factor\n")
    
    for t in target_T9_values:
        # Get the uncertainties at the specific temperature
        u_il = float(f_uncert_NACRE(t))
        u_th = float(f_uncert_th(t))
        
        # Calculate the reduction factor
        reduction_factor = u_il / u_th
        ratio = rate_med_NACRE / rate_med_th
        
        # Write the formatted data to the file, separated by tabs
        f.write(f"{t:.3f}\t{u_il:.1f}\t{u_th:.1f}\t{ratio:.1f}\n")

# Print a confirmation to the console so you know it finished successfully
print(f"Success: Uncertainty reduction table saved to '{output_filename}'")

for t in target_T9_values:
    # Get the uncertainties at the specific temperature
    u_il = float(f_uncert_NACRE(t))
    u_th = float(f_uncert_th(t))
    
    # Calculate how many times smaller the new uncertainty is
    reduction_factor = u_il / u_th
    ratio = rate_med_NACRE / rate_med_th
    
    print(f"{t:<10.3f} | {u_il:<20.1f} | {u_th:<22.1f} | {ratio:.1f}")

print("-" * 75)
print("* Uncertainty % defined as total relative width: ((High - Low) / Median) * 100")