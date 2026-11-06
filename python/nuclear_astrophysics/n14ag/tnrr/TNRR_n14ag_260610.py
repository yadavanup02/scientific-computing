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
outdir = "RatesMC_plots_260610"
os.makedirs(outdir, exist_ok=True)
plt.rcParams.update({'font.size': 10})

# --- Calculate ratio of Iliadis to This Work ---
tiny = 1e-100

if len(T9) == len(T9_iliadis) and np.allclose(T9, T9_iliadis, rtol=1e-10):
    print("\nCalculating ratio of Iliadis to This Work using direct element-wise division...")
    ratio_iliadis_to_this = rate_med_iliadis / np.clip(rate_med_this, tiny, None)
    
    # Conservative uncertainty band (Iliadis low / This high, Iliadis high / This low)
    ratio_lo_iliadis_to_this = rate_low_iliadis / np.clip(rate_hi_this, tiny, None)
    ratio_hi_iliadis_to_this = rate_hi_iliadis / np.clip(rate_low_this, tiny, None)
    T9_ratio_iliadis = T9
else:
    print("\nInterpolating Iliadis to match your work's temperature grid...")
    from scipy import interpolate
    f_ili_med = interpolate.interp1d(T9_iliadis, rate_med_iliadis, bounds_error=False, fill_value='extrapolate')
    f_ili_low = interpolate.interp1d(T9_iliadis, rate_low_iliadis, bounds_error=False, fill_value='extrapolate')
    f_ili_hi  = interpolate.interp1d(T9_iliadis, rate_hi_iliadis, bounds_error=False, fill_value='extrapolate')
    
    ratio_iliadis_to_this = f_ili_med(T9) / np.clip(rate_med_this, tiny, None)
    ratio_lo_iliadis_to_this = f_ili_low(T9) / np.clip(rate_hi_this, tiny, None)
    ratio_hi_iliadis_to_this = f_ili_hi(T9) / np.clip(rate_low_this, tiny, None)
    T9_ratio_iliadis = T9

# --- Calculate ratio of NACRE to This Work ---
def find_matching_indices(arr1, arr2, tol=1e-6):
    """Find indices where values match within tolerance"""
    idx1, idx2 = [], []
    for i, val1 in enumerate(arr1):
        diffs = np.abs(arr2 - val1)
        min_diff_idx = np.argmin(diffs)
        if diffs[min_diff_idx] < tol:
            idx1.append(i)
            idx2.append(min_diff_idx)
    return np.array(idx1), np.array(idx2)

print(f"\nFinding common points between your work and NACRE...")
idx_this_nacre, idx_nacre = find_matching_indices(T9, T9_NACRE, tol=1e-6)

if len(idx_this_nacre) > 0:
    T9_common = T9[idx_this_nacre]
    
    # Ratios for common points
    ratio_nacre_to_this = rate_med_NACRE[idx_nacre] / np.clip(rate_med_this[idx_this_nacre], tiny, None)
    ratio_lo_nacre_to_this = rate_low_NACRE[idx_nacre] / np.clip(rate_hi_this[idx_this_nacre], tiny, None)
    ratio_hi_nacre_to_this = rate_hi_NACRE[idx_nacre] / np.clip(rate_low_this[idx_this_nacre], tiny, None)
    T9_ratio_nacre = T9_common
else:
    print(f"WARNING: No common temperature points found! Interpolation logic needed.")
    ratio_nacre_to_this, ratio_lo_nacre_to_this, ratio_hi_nacre_to_this, T9_ratio_nacre = None, None, None, None

# --- Create ratio plot ---
fig, ax1 = plt.subplots(figsize=(5, 5))

# 1. Plot Baseline: This Work / This Work
ax1.plot(T9, np.ones_like(T9), color="black", lw=1.5, label="This Work")
# Uncertainty band for This Work (normalized to its own median)
ax1.fill_between(
    T9,
    rate_low_this / np.clip(rate_med_this, tiny, None),
    rate_hi_this / np.clip(rate_med_this, tiny, None),
    color="black", alpha=0.20
)

# 2. Plot ratio: Iliadis / This Work
ax1.plot(
    T9_ratio_iliadis, ratio_iliadis_to_this, color="blue", lw=1.0, label="Iliadis"
)
ax1.fill_between(
    T9_ratio_iliadis, ratio_lo_iliadis_to_this, ratio_hi_iliadis_to_this,
    color="blue", alpha=0.25
)

# 3. Plot ratio: NACRE / This Work
if T9_ratio_nacre is not None:
    ax1.plot(
        T9_ratio_nacre, ratio_nacre_to_this, color="red", lw=1.0, label="NACRE"
    )
    ax1.fill_between(
        T9_ratio_nacre, ratio_lo_nacre_to_this, ratio_hi_nacre_to_this,
        color="red", alpha=0.25
    )

# # Formatting
# ax1.set_xlabel(r"Temperature $T_9$")
# ax1.set_ylabel(r"Ratio to This Work")
# ax1.set_xscale("log")  # Standard for T9 usually, remove if you prefer linear
# ax1.legend(loc="best")
# plt.tight_layout()

# Save figure (Optional)
# plt.savefig(os.path.join(outdir, "Ratio_to_This_Work.pdf"))

# line = Line2D([0], [0], color='red', lw=1.5)
# patch = Patch(facecolor='red', alpha=0.30, edgecolor='none')
# plt.legend([(line, patch)], ["This work"])
# --- This work (red) ---
line_this   = Line2D([0], [0], color='red', lw=1.0)
patch_this  = Patch(facecolor='red', alpha=0.25, edgecolor='none')

# --- Iliadis (blue) ---
line_iliadis  = Line2D([0], [0], color='blue', lw=1.0)
patch_iliadis = Patch(facecolor='blue', alpha=0.20, edgecolor='none')

# --- NACRE (black/gray band looks nicer than solid black fill) ---
line_nacre  = Line2D([0], [0], color='black', lw=1.0)
patch_nacre = Patch(facecolor='black', alpha=0.25, edgecolor='none')

# Combine into legend
# place legend at an exact (x,y) in axes coordinates:
legend = ax1.legend(
    [(line_this, patch_this),
     (line_iliadis, patch_iliadis),
     (line_nacre, patch_nacre)],
    ["This work", "Iliadis", "NACRE"],
    loc="upper right",
    bbox_to_anchor=(0.9, 0.99),  # exact position in axis coords (x, y)
    fontsize=12,
    frameon=True
)

# -- alternative: use figure coordinates by supplying bbox_transform --
# legend = ax1.legend(..., loc="upper left", bbox_to_anchor=(0.92, 0.12),
#                     bbox_transform=plt.gcf().transFigure, fontsize=12, frameon=True)


# Plot formatting
# plt.xscale("log")
ax1.set_yscale("log")
# plt.xlim(0.07, 2.0)
ax1.set_ylim(0.08, 10.0)
# plt.xlim(0.01, 10.0)
ax1.set_xlim(0.08, 1.5)
# plt.ylim(0.0, 1.2)
# plt.xticks([0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 2.0], [f"{x:.1f}" for x in [0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 2.0]])
#plt.yticks([0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0], [f"{y:.1f}" for y in [0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0]])
# plt.yticks([0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2], [f"{y:.1f}" for y in [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2]])
ax1.set_xlabel("Temperature [GK]", fontsize=13)
# Option A: simple offset from the axis using labelpad (points)
ax1.set_ylabel("Ratio to This Work", fontsize=13, labelpad=0)

# Option B: explicit axis coordinates (axes fraction units: x,y in [0,1])
# (x < 0 moves the label further left; y controls vertical position)
# ax1.yaxis.set_label_coords(-0.08, 0.5)
# plt.title("Comparison of Reaction Rates", fontsize=16)
ax1.tick_params(axis='both', which='major', labelsize=12)
ax1.tick_params(axis='both', which='minor', labelsize=12)
# plt.legend(fontsize=13)
# plt.legend(fontsize=14, ncol=1, loc="upper right")

# plt.text(0.15, 0.9, "(b)", transform=plt.gca().transAxes, fontsize=16, verticalalignment='top', horizontalalignment='right')  


# Customize legend
# legend = plt.legend(loc='best', fontsize=11, framealpha=0.9)
# legend.get_frame().set_edgecolor('black')

# ax1.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3)
ax1.tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3)
#plt.grid(True, alpha=0.3, ls='--')

# plt.tight_layout()
# plt.savefig(os.path.join(outdir, "Ratio_this_to_NACRE_v1.pdf"), dpi=300)
# plt.close()

print(f"\n=== Plot saved as {os.path.join(outdir, 'Ratio_this_to_iliadis_NACRE.pdf')} ===")
print(f"Ratio statistics with Iliadis:")
print(f"  Min ratio: {np.min(ratio_iliadis_to_this):.3f}")
print(f"  Max ratio: {np.max(ratio_iliadis_to_this):.3f}")
print(f"  Mean ratio: {np.mean(ratio_iliadis_to_this):.3f}")

if len(idx_this_nacre) > 0:
    print(f"\nRatio statistics with NACRE (common points):")
    print(f"  Min ratio: {np.min(ratio_nacre_to_this):.3f}")
    print(f"  Max ratio: {np.max(ratio_nacre_to_this):.3f}")
    print(f"  Mean ratio: {np.mean(ratio_nacre_to_this):.3f}")

# # ---------- 2) Fraction of each resonance: this work vs literature (dashed = lit) ----------#
# # Load contribution files
this_cont = pd.read_csv("RatesAnup_final/RatesMC.cont", delim_whitespace=True, comment="#", header=None)
T9_cont = this_cont.iloc[:, 0].values
rates_DC = this_cont.iloc[:,2].values
rates_580 = this_cont.iloc[:,8].values
rates_1136 = this_cont.iloc[:,11].values
rates_1395 = this_cont.iloc[:,14].values
rates_1530 = this_cont.iloc[:,17].values
rates_1618 = this_cont.iloc[:,20].values
rates_1768 = this_cont.iloc[:,23].values
rates_2161 = this_cont.iloc[:,26].values
rates_2177 = this_cont.iloc[:,29].values
rates_2348 = this_cont.iloc[:,32].values
rates_2350 = this_cont.iloc[:,35].values
rates_305 = this_cont.iloc[:,56].values

iliadis_cont = pd.read_csv("RatesIliadis_final/RatesMC.cont", delim_whitespace=True, comment="#", header=None)
T9_cont_iliadis = iliadis_cont.iloc[:, 0].values
rates_DC_iliadis = iliadis_cont.iloc[:,2].values
rates_580_iliadis = iliadis_cont.iloc[:,8].values
rates_1136_iliadis = iliadis_cont.iloc[:,11].values
rates_1395_iliadis = iliadis_cont.iloc[:,14].values
rates_1530_iliadis = iliadis_cont.iloc[:,17].values
rates_1618_iliadis = iliadis_cont.iloc[:,20].values
rates_1768_iliadis = iliadis_cont.iloc[:,23].values
rates_2161_iliadis = iliadis_cont.iloc[:,26].values
rates_2177_iliadis = iliadis_cont.iloc[:,29].values
rates_2348_iliadis = iliadis_cont.iloc[:,32].values
rates_2350_iliadis = iliadis_cont.iloc[:,35].values
rates_305_iliadis = iliadis_cont.iloc[:,56].values

ax2 = ax1.twinx()
#plt.figure(figsize=(6,5))
# plt.plot(T9_cont, rates_305, color="C2", label="E$_\\alpha= 305$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_305_iliadis, color="C2", linestyle = "--", label="", linewidth=1.0)
ax2.plot(T9_cont, rates_580, color="C3", label="E$_\\alpha= 580$ keV", linewidth=1.5, linestyle=':')
#plt.plot(T9_cont_iliadis, rates_580_iliadis, color="C0", linestyle = "--", label="", linewidth=1.0)
ax2.plot(T9_cont, rates_1136, color="C0", label="E$_\\alpha= 1136$ keV", linewidth=1.5, linestyle=':')
#plt.plot(T9_cont_iliadis, rates_1530_iliadis, color="C3", linestyle = "--", label="", linewidth=1.0)
ax2.plot(T9_cont, rates_1618, color="C4", label="E$_\\alpha= 1618$ keV", linewidth=1.5, linestyle=':')
ax2.set_ylim(0,2.0)
# plt.xlabel("Temperature [GK]", fontsize=14); 
ax2.set_ylabel("Fractional contribution", fontsize=13)
# #plt.grid(True, ls="--", lw=0.4)
ax2.legend(fontsize=12, ncol=1, loc="upper right",
           bbox_to_anchor=(0.99, 0.50))
# plt.tight_layout()
# plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3)
ax2.tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3, labelsize=12)
# plt.savefig(os.path.join(outdir, "TNRR_combined.pdf"))
# plt.close()
# Create two separate legends and place them at different locations
ax = plt.gca()

# Save updated figure
plt.savefig(os.path.join(outdir, "TNRR_and_FC_v1.pdf"))
plt.close()