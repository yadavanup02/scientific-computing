#!/usr/bin/env python3
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd
import os
from matplotlib.lines import Line2D
from matplotlib.patches import Patch

# --- Load data files ---
# Load Iliadis 2010 rate file
iliadis = pd.read_csv("RatesIliadis_final/RatesMC.out", delim_whitespace=True, comment="#", header=None)
T9_iliadis = iliadis.iloc[:, 0].values
rate_low_iliadis  = iliadis.iloc[:, 1].values
rate_med_iliadis = iliadis.iloc[:, 2].values
rate_hi_iliadis  = iliadis.iloc[:, 3].values

# Load this work rate file
this = pd.read_csv("RatesAnup_final/RatesMC.out", delim_whitespace=True, comment="#", header=None)
T9 = this.iloc[:, 0].values
rate_low_this = this.iloc[:, 1].values
rate_med_this = this.iloc[:, 2].values
rate_hi_this = this.iloc[:, 3].values

# Load NACRE data from Gorres200.txt
Gor = pd.read_csv("Gorres200.txt", delim_whitespace=True, comment="#", header=0)
T9_NACRE = Gor.iloc[:, 0].values
rate_med_NACRE = Gor.iloc[:, 5].values
rate_low_NACRE = Gor.iloc[:, 6].values
rate_hi_NACRE = Gor.iloc[:, 7].values

# --- Diagnostic information ---
print("\n=== Data size diagnostics ===")
print(f"Your work: T9 size = {len(T9)}, rate size = {len(rate_med_this)}")
print(f"Iliadis: T9 size = {len(T9_iliadis)}, rate size = {len(rate_med_iliadis)}")
print(f"NACRE (from Gorres200.txt): T9 size = {len(T9_NACRE)}, rate size = {len(rate_med_NACRE)}")

# Check if your work and Iliadis have the same temperature values
if len(T9) == len(T9_iliadis):
    max_diff = np.max(np.abs(T9 - T9_iliadis))
    print(f"\nYour work and Iliadis have same number of points.")
    print(f"Maximum temperature difference: {max_diff:.2e}")
    if max_diff < 1e-10:
        print("Temperature arrays are identical.")
    else:
        print("Temperature arrays differ slightly.")
else:
    print(f"\nWARNING: Your work and Iliadis have different number of points!")

# --- Output directory for plots ---
outdir = "RatesMC_plots_final"
os.makedirs(outdir, exist_ok=True)
plt.rcParams.update({'font.size': 10})

# --- Calculate ratio with Iliadis ---
# Since they have same number of points, we can calculate directly
# But first check if temperatures match
tiny = 1e-100
if len(T9) == len(T9_iliadis) and np.allclose(T9, T9_iliadis, rtol=1e-10):
    print("\nCalculating ratio with Iliadis using direct element-wise division...")
    ratio_this_to_iliadis = rate_med_this / np.clip(rate_med_iliadis, tiny, None)
    T9_ratio_iliadis = T9
else:
    print("\nInterpolating Iliadis to match your work's temperature grid...")
    from scipy import interpolate
    # Interpolate Iliadis to your T9 grid
    f_iliadis = interpolate.interp1d(T9_iliadis, rate_med_iliadis, 
                                    bounds_error=False, fill_value='extrapolate')
    rate_med_iliadis_interp = f_iliadis(T9)
    ratio_this_to_iliadis = rate_med_this / np.clip(rate_med_iliadis_interp, tiny, None)
    T9_ratio_iliadis = T9

# --- Calculate ratio with NACRE ---
# Function to find matching indices with tolerance
def find_matching_indices(arr1, arr2, tol=1e-6):
    """Find indices where values match within tolerance"""
    idx1 = []
    idx2 = []
    for i, val1 in enumerate(arr1):
        # Find the closest value in arr2
        diffs = np.abs(arr2 - val1)
        min_diff_idx = np.argmin(diffs)
        if diffs[min_diff_idx] < tol:
            idx1.append(i)
            idx2.append(min_diff_idx)
    return np.array(idx1), np.array(idx2)

print(f"\nFinding common points between your work and NACRE...")
print(f"Your work T9 range: {T9.min():.3f} to {T9.max():.3f}")
print(f"NACRE T9 range: {T9_NACRE.min():.3f} to {T9_NACRE.max():.3f}")

idx_this_nacre, idx_nacre = find_matching_indices(T9, T9_NACRE, tol=1e-6)

if len(idx_this_nacre) > 0:
    print(f"Found {len(idx_this_nacre)} common temperature points.")
    # Get the actual temperature values to verify
    common_T9_values = T9[idx_this_nacre]
    print(f"Common temperatures: {common_T9_values}")
    
    # Calculate ratio for common points
    ratio_this_to_NACRE = rate_med_this[idx_this_nacre] / np.clip(rate_med_NACRE[idx_nacre], tiny, None)
    T9_ratio_nacre = common_T9_values
    
    ratio_lo_this_to_NACRE = rate_low_this[idx_this_nacre] / np.clip(rate_hi_NACRE[idx_nacre], tiny, None)
    ratio_hi_this_to_NACRE = rate_hi_this[idx_this_nacre] / np.clip(rate_low_NACRE[idx_nacre], tiny, None)
    
    # Also calculate interpolated version for comparison
    from scipy import interpolate
    f_nacre = interpolate.interp1d(T9_NACRE, rate_med_NACRE, 
                                  bounds_error=False, fill_value=np.nan)
    rate_med_NACRE_interp = f_nacre(T9)
    ratio_this_to_NACRE_interp = rate_med_this / np.clip(rate_med_NACRE_interp, tiny, None)
else:
    print(f"WARNING: No common temperature points found! Using interpolation.")
    from scipy import interpolate
    f_nacre = interpolate.interp1d(T9_NACRE, rate_med_NACRE, 
                                  bounds_error=False, fill_value=np.nan)
    rate_med_NACRE_interp = f_nacre(T9)
    ratio_this_to_NACRE_interp = rate_med_this / np.clip(rate_med_NACRE_interp, tiny, None)
    T9_ratio_nacre = T9
    ratio_this_to_NACRE = ratio_this_to_NACRE_interp

# Ratio of Iliadis to NACRE for reference
if len(idx_this_nacre) > 0:
    T9_common = T9[idx_this_nacre]
    ratio_iliadis_to_NACRE = rate_med_iliadis[idx_this_nacre] / np.clip(rate_med_NACRE[idx_nacre], tiny, None)
    T9_ratio_iliadis_nacre = T9_common
    ratio_lo_iliadis_to_NACRE = rate_low_iliadis[idx_this_nacre] / np.clip(rate_hi_NACRE[idx_nacre], tiny, None)
    ratio_hi_iliadis_to_NACRE = rate_hi_iliadis[idx_this_nacre] / np.clip(rate_low_NACRE[idx_nacre], tiny, None)
else:
    ratio_iliadis_to_NACRE = None
    T9_ratio_iliadis_nacre = None
    ratio_lo_iliadis_to_NACRE = None
    ratio_hi_iliadis_to_NACRE = None

# --- conservative uncertainty band ---
ratio_lo = rate_low_this / np.clip(rate_hi_iliadis, tiny, None)
ratio_hi = rate_hi_this / np.clip(rate_low_iliadis, tiny, None)

# --- Create ratio plot ---
plt.figure(figsize=(6, 5))

#          label=r"$\frac{\langle\sigma v\rangle_{\mathrm{this}}}{\langle\sigma v\rangle_{\mathrm{Iliadis}}}$")
# plt.fill_between(T9, ratio_lo, ratio_hi, color="blue", alpha=0.10,
#                  label="") #Uncertainty band
# Plot ratio with NACRE
# if len(idx_this_nacre) > 0:

    # Also plot the interpolated version as a line
    # plt.plot(T9, ratio_this_to_NACRE_interp, 'r--', lw=0.8, alpha=0.5,
    #          label=r"$\frac{\langle\sigma v\rangle_{\mathrm{this}}}{\langle\sigma v\rangle_{\mathrm{NACRE}}}$ (interpolated)")
# else:
#     plt.plot(T9_ratio_nacre, ratio_this_to_NACRE, 'r-', lw=1.5,
#              label=r"$\frac{\langle\sigma v\rangle_{\mathrm{this}}}{\langle\sigma v\rangle_{\mathrm{NACRE}}}$ (interpolated)")
# Plot NACRE/NACRE ratio (should be 1, but shows NACRE uncertainty band)
plt.plot(T9_ratio_nacre, np.ones_like(T9_ratio_nacre), color="black", lw=1.0) #label=r"$\frac{\langle\sigma v\rangle_{\mathrm{NACRE}}}{\langle\sigma v\rangle_{\mathrm{NACRE}}}$
plt.fill_between(
    T9_ratio_nacre,
    rate_low_NACRE[idx_nacre] / np.clip(rate_hi_NACRE[idx_nacre], tiny, None),
    rate_hi_NACRE[idx_nacre] / np.clip(rate_low_NACRE[idx_nacre], tiny, None),
    color="black", alpha=0.30, label="NACRE"
)
# Plot ratio with Iliadis (line and band with single legend entry)
line_iliadis, = plt.plot(
    T9_ratio_iliadis_nacre, ratio_iliadis_to_NACRE, color="blue", lw=1.0,
    label="Iliadis"
)
band_iliadis = plt.fill_between(
    T9_ratio_iliadis_nacre, ratio_lo_iliadis_to_NACRE, ratio_hi_iliadis_to_NACRE,
    color="blue", alpha=0.30
)
# Remove band from legend, keep only the line
# band_iliadis.set_label(None)
# plt.plot(T9_ratio_nacre, ratio_this_to_NACRE, 'r-', markersize=0, lw=1.0,
#              label="This work") #$\frac{\langle\sigma v\rangle_{\mathrm{This}}}{\langle\sigma v\rangle_{\mathrm{NACRE}}}$

# plt.fill_between(T9_ratio_nacre, ratio_lo_this_to_NACRE, ratio_hi_this_to_NACRE, color="red", alpha=0.30,
#                  label="") #Uncertainty band
plt.plot(T9_ratio_nacre, ratio_this_to_NACRE,
         color='red', lw=1.0, label=None)

band_iliadis = plt.fill_between(
    T9_ratio_nacre,
    ratio_lo_this_to_NACRE,
    ratio_hi_this_to_NACRE,
    color='red',
    alpha=0.30,
    label="This work"
)

# line = Line2D([0], [0], color='red', lw=1.5)
# patch = Patch(facecolor='red', alpha=0.30, edgecolor='none')
# plt.legend([(line, patch)], ["This work"])
# --- This work (red) ---
line_this   = Line2D([0], [0], color='red', lw=1.5)
patch_this  = Patch(facecolor='red', alpha=0.30, edgecolor='none')

# --- Iliadis (blue) ---
line_iliadis  = Line2D([0], [0], color='blue', lw=1.5)
patch_iliadis = Patch(facecolor='blue', alpha=0.30, edgecolor='none')

# --- NACRE (black/gray band looks nicer than solid black fill) ---
line_nacre  = Line2D([0], [0], color='black', lw=1.5)
patch_nacre = Patch(facecolor='black', alpha=0.35, edgecolor='none')

# Combine into legend
plt.legend(
    [(line_this, patch_this),
     (line_iliadis, patch_iliadis),
     (line_nacre, patch_nacre)],
    ["This work", "Iliadis", "NACRE"],
    loc="lower right", fontsize=14, frameon=True
)


# plt.plot(
#     T9_ratio_nacre,
#     rate_low_NACRE[idx_nacre] / np.clip(rate_hi_NACRE[idx_nacre], tiny, None),
#     color="black", linestyle=":", label=""
# )
# plt.plot(
#     T9_ratio_nacre,
#     rate_hi_NACRE[idx_nacre] / np.clip(rate_low_NACRE[idx_nacre], tiny, None),
#     color="black", linestyle=":", label=""
# )
# Add horizontal reference line at 1
# plt.axhline(y=1.0, color='k', linestyle='--', alpha=0.5, lw=0.8)

# Add shaded area for helium burning temperature range
# plt.fill_betweenx(y=[0.015, 0.02], x1=0.1, x2=0.4, color='green', alpha=0.4, 
#                   label="")

# Add vertical lines at key temperature points
# plt.axvline(x=0.1, color='gray', linestyle=':', alpha=0.5, lw=0.5)
# plt.axvline(x=0.4, color='gray', linestyle=':', alpha=0.5, lw=0.5)

# Plot formatting
plt.xscale("log")
plt.yscale("log")
# plt.xlim(0.07, 2.0)
plt.ylim(0.01, 10.0)
# plt.xlim(0.01, 10.0)
plt.xlim(0.08, 2.0)
# plt.ylim(0.0, 1.2)
# plt.xticks([0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 2.0], [f"{x:.1f}" for x in [0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 2.0]])
# plt.yticks([0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0], [f"{y:.1f}" for y in [0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0]])
# plt.yticks([0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2], [f"{y:.1f}" for y in [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2]])    
plt.xlabel("Temperature [GK]", fontsize=14)
plt.ylabel("Ratio to NACRE", fontsize=14)
# plt.title("Comparison of Reaction Rates", fontsize=16)
plt.tick_params(axis='both', which='major', labelsize=13)
plt.tick_params(axis='both', which='minor', labelsize=13)
# plt.legend(fontsize=13)
# plt.legend(fontsize=14, ncol=1, loc="upper right")

plt.text(0.15, 0.9, "(b)", transform=plt.gca().transAxes, fontsize=16, verticalalignment='top', horizontalalignment='right')  


# Customize legend
# legend = plt.legend(loc='best', fontsize=11, framealpha=0.9)
# legend.get_frame().set_edgecolor('black')

plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3)
#plt.grid(True, alpha=0.3, ls='--')

plt.tight_layout()
plt.savefig(os.path.join(outdir, "Ratio_this_to_NACRE_v1.pdf"), dpi=300)
plt.close()

print(f"\n=== Plot saved as {os.path.join(outdir, 'Ratio_this_to_iliadis_NACRE.pdf')} ===")
print(f"Ratio statistics with Iliadis:")
print(f"  Min ratio: {np.min(ratio_this_to_iliadis):.3f}")
print(f"  Max ratio: {np.max(ratio_this_to_iliadis):.3f}")
print(f"  Mean ratio: {np.mean(ratio_this_to_iliadis):.3f}")

if len(idx_this_nacre) > 0:
    print(f"\nRatio statistics with NACRE (common points):")
    print(f"  Min ratio: {np.min(ratio_this_to_NACRE):.3f}")
    print(f"  Max ratio: {np.max(ratio_this_to_NACRE):.3f}")
    print(f"  Mean ratio: {np.mean(ratio_this_to_NACRE):.3f}")