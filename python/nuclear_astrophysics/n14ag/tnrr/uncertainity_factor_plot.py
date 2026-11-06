# import matplotlib.pyplot as plt
# import numpy as np

# # --- Data from your output ---
# T9 = np.array([0.050, 0.080, 0.100, 0.200, 0.400, 0.500, 1.000, 2.000])
# uncert_iliadis = np.array([143.8, 238.9, 49.2, 27.3, 17.8, 16.0, 9.0, 10.8])
# uncert_this_work = np.array([148.0, 292.8, 14.4, 12.9, 12.7, 12.3, 7.8, 10.6])

# # --- Formatting for PRL ---
# plt.rcParams.update({
#     'font.size': 12,
#     'font.family': 'serif',
#     'axes.labelsize': 14,
#     'xtick.labelsize': 12,
#     'ytick.labelsize': 12,
#     'legend.fontsize': 12,
# })

# fig, ax = plt.subplots(figsize=(6, 4.5))

# # Plot the uncertainties
# ax.plot(T9, uncert_iliadis, marker='o', linestyle='--', color='#0077BB', linewidth=2, label='Iliadis (2010)')
# ax.plot(T9, uncert_this_work, marker='s', linestyle='-', color='#CC3311', linewidth=2, label='Present Work')

# # Highlight the Gamow window (0.1 to 0.4 GK)
# ax.axvspan(0.1, 0.4, color='gray', alpha=0.2, label='Gamow Window')

# # Formatting the axes
# ax.set_xscale('log')
# ax.set_yscale('log') # Log scale helps visualize the massive drop at T9=0.1

# ax.set_xlabel('Temperature $T_9$ (GK)')
# ax.set_ylabel('Fractional Uncertainty (%)')

# # Restrict axes to focus on the relevant physics
# ax.set_xlim(0.04, 2.5)
# ax.set_ylim(5, 400)

# ax.legend(frameon=False, loc='upper right')
# plt.tight_layout()

# # Save as a high-res PDF for LaTeX
# plt.savefig("Uncertainty_Reduction.pdf", format='pdf', dpi=300)
# plt.show()
import pandas as pd
import numpy as np
import matplotlib as plt
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


# --- 3. Calculate the Ratios ---
# Normalize everything by the Iliadis central rate
# 1. Iliadis (Reference)
ratio_il_med = rate_med_il / rate_med_il  
ratio_il_low = rate_hi_il / rate_med_il
ratio_il_hi  = rate_low_il / rate_med_il

# 2. Present Work
ratio_th_med = rate_med_th / rate_med_th
ratio_th_low = rate_low_th / rate_med_th
ratio_th_hi  = rate_hi_th / rate_med_th

# --- 4. Create the PRL-Formatted Plot ---
import matplotlib.pyplot as plt
fig, ax = plt.subplots(figsize=(7, 5))

# Plot Iliadis 2010 (The Baseline)
ax.plot(T9_th, ratio_il_med, color='#0077BB', linestyle='--', linewidth=1.0, label='')
ax.fill_between(T9_th, ratio_il_low, ratio_il_hi, color='#0077BB', alpha=0.2, label='Iliadis')

# Plot Present Work
ax.plot(T9_th, ratio_th_med, color='#CC3311', linestyle='-', linewidth=1.0, label='')
ax.fill_between(T9_th, ratio_th_low, ratio_th_hi, color='#CC3311', alpha=0.4, label='Present Work')

# # Highlight the Gamow window
# ax.axvspan(0.1, 0.4, color='gray', alpha=0.15, label='Gamow Window')

# Formatting
ax.set_xscale('log')
# ax.set_yscale('log') # Log scale is usually best for these ratios, switch to 'linear' if the bands are tight

ax.set_xlabel(' T (GK)')
ax.set_ylabel('Reaction Rate Ratio')

# Set limits to focus on the physics
ax.set_xlim(0.05, 2.5)
ax.set_ylim(0.1, 1.9)
# ax.set_ylim(0.1, 10) # Uncomment and adjust these Y-limits depending on how large the ratio gets

ax.legend(frameon=False, loc='upper right')
plt.tight_layout()

# Save for LaTeX
plt.savefig("Rate_Ratio_Comparison.pdf", format='pdf', dpi=300)
plt.show()