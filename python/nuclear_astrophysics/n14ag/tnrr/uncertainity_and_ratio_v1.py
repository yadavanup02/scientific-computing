import pandas as pd
import numpy as np

# --- 1. Load Data ---
# Load Iliadis 2010 rate file
iliadis = pd.read_csv("RatesIliadis_final/RatesMC.out", delim_whitespace=True, comment="#", header=4)
T9_il = iliadis.iloc[:, 0].values
rate_low_il = iliadis.iloc[:, 1].values
rate_med_il = iliadis.iloc[:, 2].values
rate_hi_il  = iliadis.iloc[:, 3].values

# Load This Work rate file
this = pd.read_csv("RatesAnup-260703/RatesMC.out", delim_whitespace=True, comment="#", header=4)
T9_th = this.iloc[:, 0].values
rate_low_th = this.iloc[:, 1].values
rate_med_th = this.iloc[:, 2].values
rate_hi_th  = this.iloc[:, 3].values

# Load NACRE data from Gorres200.txt
Gor = pd.read_csv("Gorres200.txt", delim_whitespace=True, comment="#", header=None)
T9_nacre = Gor.iloc[:, 0].values
rate_med_nacre = Gor.iloc[:, 5].values
rate_low_nacre = Gor.iloc[:, 6].values
rate_hi_nacre = Gor.iloc[:, 7].values

# --- 2. Calculate Fractional Uncertainties (Percentage) ---
uncert_il = ((rate_hi_il - rate_low_il) / rate_med_il) * 100
uncert_th = ((rate_hi_th - rate_low_th) / rate_med_th) * 100
uncert_nacre = ((rate_hi_nacre - rate_low_nacre) / rate_med_nacre) * 100

# --- 3. Define Target Temperatures ---
target_T9_values = [
0.010, 0.011, 0.012, 0.013, 0.014, 0.015, 0.016, 0.018, 0.020, 0.025, 
0.030, 0.040, 0.050, 0.060, 0.070, 0.080, 0.090, 0.100, 0.110, 0.120, 
0.130, 0.140, 0.150, 0.160, 0.180, 0.200, 0.250, 0.300, 0.350, 0.400, 
0.450, 0.500, 0.600, 0.700, 0.800, 0.900, 1.000, 1.250, 1.500, 1.750, 
2.000, 2.500, 3.000, 3.500, 4.000, 5.000, 6.000, 7.000, 8.000, 9.000, 10.000
]
output_filename = "Rates_ratios_260708.txt"

# --- 4. Print & Write ---
print("\n=== Reaction Rate Ratios & Uncertainties ===")
# Expanded headers for the console
print(f"{'T9 (GK)':<8} | {'Ratio (Il/Th)':<13} | {'Ratio (NACRE/Th)':<16} | {'Red. (Il)':<9} | {'Red. (NACRE)':<12} | {'Uncert Il(%)':<12} | {'Uncert NACRE(%)':<15} | {'Uncert Th(%)'}")
print("-" * 115)

with open(output_filename, 'w') as f:
    # Expanded headers for the text file
    f.write("T9 (GK)\tRatio (Iliadis/This)\tRatio (NACRE/This)\tRed. Factor (Iliadis)\tRed. Factor (NACRE)\tUncertainty NACRE (%)\tUncertainty Iliadis (%)\tUncertainty This (%)\n")

    for t in target_T9_values:
        # --- Direct Array Indexing for RatesMC Grids ---
        # Find the index of the temperature closest to 't' in the main grid
        idx_main = np.argmin(np.abs(T9_th - t))
        
        # Extract raw values directly using the index
        u_il = uncert_il[idx_main]
        u_th = uncert_th[idx_main]
        r_il = rate_med_il[idx_main]
        r_th = rate_med_th[idx_main]
        
        red_factor_il = u_il / u_th if u_th != 0 else 0
        ratio_il = r_il / r_th if r_th != 0 else 0
        
        # --- Strict Match Check for NACRE ---
        idx_match_nacre = np.where(np.isclose(T9_nacre, t, atol=1e-5))[0]
        
        if len(idx_match_nacre) > 0:
            idx_nacre = idx_match_nacre[0]
            u_nacre_val = uncert_nacre[idx_nacre]
            r_nacre_val = rate_med_nacre[idx_nacre]
            
            red_factor_nacre = u_nacre_val / u_th if u_th != 0 else 0
            ratio_nacre = r_nacre_val / r_th if r_th != 0 else 0
            
            str_ratio_nacre = f"{ratio_nacre:.1f}"
            str_red_nacre = f"{red_factor_nacre:.2f}"
            str_u_nacre = f"{u_nacre_val:.1f}"
        else:
            str_ratio_nacre = "----"
            str_red_nacre = "----"
            str_u_nacre = "----"

        # Write all columns to file
        f.write(f"{t:.3f}\t{ratio_il:.2f}\t{str_ratio_nacre}\t{red_factor_il:.2f}\t{str_red_nacre}\t{str_u_nacre}\t{u_il:.1f}\t{u_th:.1f}\n")

        # Print all columns to console
        print(f"{t:<8.3f} | {ratio_il:<13.1f} | {str_ratio_nacre:<16} | {red_factor_il:<9.2f} | {str_red_nacre:<12} | {u_il:<12.1f} | {str_u_nacre:<15} | {u_th:.1f}")

print("-" * 115)
print(f"Success: Comparison table saved to '{output_filename}'")