#!/usr/bin/env python3
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd
import os
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
def tol_cset(colorset=None):
    from collections import namedtuple
    
    namelist = ('bright', 'high-contrast', 'vibrant', 'muted', 'medium-contrast', 'light')
    if colorset is None:
        return namelist
    if colorset not in namelist:
        colorset = 'bright'
        print('*** Warning: requested colorset not defined,',
              'known colorsets are {}.'.format(namelist),
              'Using {}.'.format(colorset))
    if colorset == 'vibrant':
            cset = namedtuple('Vcset',
                        'orange blue cyan magenta red teal grey black')
            return cset('#EE7733', '#0077BB', '#33BBEE', '#EE3377', '#CC3311',
                        '#009988', '#BBBBBB', '#000000')

# plt.rcParams.update({
#     "text.usetex": True,
#     "font.family": "serif",
#     "font.serif": ["Computer Modern Roman"],
#     "font.size": 12,
#     "axes.labelsize": 13,
#     "axes.titlesize": 13,
#     "legend.fontsize": 12,
#     "xtick.labelsize": 12,
#     "ytick.labelsize": 12,
# })
plt.rcParams.update({
    "font.family": "serif",
    "mathtext.fontset": "cm",
    "font.size": 12,
})
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
outdir = "RatesMC_plots_260611"
os.makedirs(outdir, exist_ok=True)
plt.rcParams.update({'font.size': 10})

tiny = 1e-100

# ---------------------------------------------------------
# Iliadis / This Work
# ---------------------------------------------------------
if len(T9) == len(T9_iliadis) and np.allclose(T9, T9_iliadis, rtol=1e-10):

    ratio_iliadis_to_this = (
        rate_med_iliadis /
        np.clip(rate_med_this, tiny, None)
    )

    ratio_lo_iliadis_to_this = (
        rate_low_iliadis /
        np.clip(rate_hi_this, tiny, None)
    )

    ratio_hi_iliadis_to_this = (
        rate_hi_iliadis /
        np.clip(rate_low_this, tiny, None)
    )

    T9_ratio_iliadis = T9

else:

    from scipy import interpolate

    f_iliadis_low = interpolate.interp1d(
        T9_iliadis, rate_low_iliadis,
        bounds_error=False, fill_value="extrapolate"
    )

    f_iliadis_med = interpolate.interp1d(
        T9_iliadis, rate_med_iliadis,
        bounds_error=False, fill_value="extrapolate"
    )

    f_iliadis_hi = interpolate.interp1d(
        T9_iliadis, rate_hi_iliadis,
        bounds_error=False, fill_value="extrapolate"
    )

    ratio_iliadis_to_this = (
        f_iliadis_med(T9) /
        np.clip(rate_med_this, tiny, None)
    )

    ratio_lo_iliadis_to_this = (
        f_iliadis_low(T9) /
        np.clip(rate_hi_this, tiny, None)
    )

    ratio_hi_iliadis_to_this = (
        f_iliadis_hi(T9) /
        np.clip(rate_low_this, tiny, None)
    )

    T9_ratio_iliadis = T9

def find_matching_indices(arr1, arr2, tol=1e-6):
    idx1 = []
    idx2 = []

    for i, val1 in enumerate(arr1):
        diffs = np.abs(arr2 - val1)
        min_diff_idx = np.argmin(diffs)

        if diffs[min_diff_idx] < tol:
            idx1.append(i)
            idx2.append(min_diff_idx)

    return np.array(idx1), np.array(idx2)
# ---------------------------------------------------------
# Common temperatures with NACRE
# ---------------------------------------------------------
idx_this_nacre, idx_nacre = find_matching_indices(
    T9,
    T9_NACRE,
    tol=1e-6
)

if len(idx_this_nacre) == 0:
    raise RuntimeError("No common temperatures found with NACRE")


T9_ratio_nacre = T9[idx_this_nacre]


# ---------------------------------------------------------
# NACRE / This Work
# ---------------------------------------------------------
ratio_NACRE_to_this = (
    rate_med_NACRE[idx_nacre]
    / np.clip(rate_med_this[idx_this_nacre], tiny, None)
)

ratio_lo_NACRE_to_this = (
    rate_low_NACRE[idx_nacre]
    / np.clip(rate_hi_this[idx_this_nacre], tiny, None)
)

ratio_hi_NACRE_to_this = (
    rate_hi_NACRE[idx_nacre]
    / np.clip(rate_low_this[idx_this_nacre], tiny, None)
)


# ---------------------------------------------------------
# This Work / This Work
# ---------------------------------------------------------
ratio_this_to_this = np.ones_like(T9)

ratio_lo_this_to_this = (
    rate_low_this
    / np.clip(rate_hi_this)
)

ratio_hi_this_to_this = (
    rate_hi_this
    / np.clip(rate_low_this)
)

# --- Create ratio plot ---
plt.figure(figsize=(3, 6))
fig, ax = plt.subplots()


# ----------------------------------
# NACRE / THIS
# ----------------------------------
ax.plot(
    T9_ratio_nacre,
    ratio_NACRE_to_this,
    color="#000000",
    lw=1.0
)

ax.fill_between(
    T9_ratio_nacre,
    ratio_lo_NACRE_to_this,
    ratio_hi_NACRE_to_this,
    color="#000000",
    alpha=0.30,
    edgecolor='none',
    linewidth=0
)

# ----------------------------------
# ILIADIS / THIS
# ----------------------------------
ax.plot(
    T9_ratio_iliadis,
    ratio_iliadis_to_this,
    color="#0711C5D3",
    lw=1.0
)

ax.fill_between(
    T9_ratio_iliadis,
    ratio_lo_iliadis_to_this,
    ratio_hi_iliadis_to_this,
    color="#0711C5D3",
    alpha=0.40, 
    edgecolor='none',
    linewidth=0
)

# ----------------------------------
# THIS WORK (reference)
# ----------------------------------
ax.plot(
    T9,
    ratio_this_to_this,
    color="#910808BD",
    lw=1.0
)

ax.fill_between(
    T9,
    ratio_lo_this_to_this,
    ratio_hi_this_to_this,
    color="#E90D0DBC",
    alpha=0.40,
    edgecolor='none',
    linewidth=0
) ##CC3311
# line = Line2D([0], [0], color='red', lw=1.5)
# patch = Patch(facecolor='red', alpha=0.30, edgecolor='none')
# plt.legend([(line, patch)], ["This work"])
# --- This work (black) ---
# --- NACRE (black/gray band looks nicer than solid black fill) ---
line_nacre  = Line2D([0], [0], color='#000000', lw=1.0)
patch_nacre = Patch(facecolor='#000000', alpha=0.30, edgecolor='none')

# --- Iliadis (blue) ---
line_iliadis  = Line2D([0], [0], color='#0711C5D3', lw=1.0)
patch_iliadis = Patch(facecolor='#0711C5D3', alpha=0.40, edgecolor='none')

line_this   = Line2D([0], [0], color='#E90D0DBC', lw=1.0)
patch_this  = Patch(facecolor='#E90D0DBC', alpha=0.40, edgecolor='none')


# Combine into legend
# place legend at an exact (x,y) in axes coordinates:
legend1 = ax.legend(
    [(line_nacre, patch_nacre),
     (line_iliadis, patch_iliadis),
     (line_this, patch_this)],
    ["NACRE", "Iliadis", "This Work"],
    loc="upper right",
    bbox_to_anchor=(0.98, 0.99),  # exact position in axis coords (x, y)
    fontsize=12,
    frameon=True
)
# simple line-only legend for the resonance traces
# line_580 = Line2D([0], [0], color="#009988", lw=1.0, linestyle='--')
# line_1136 = Line2D([0], [0], color="#e69f00", lw=1.0, linestyle='--')
# line_1618 = Line2D([0], [0], color="#0b77b6", lw=1.0, linestyle='--')
# line_total = Line2D([0], [0], color="#BBBBBB", lw=1.0, linestyle='--')

# legend1 = ax.legend(
#     [(line_nacre, patch_nacre),
#      (line_iliadis, patch_iliadis),
#      (line_this, patch_this), line_580, line_1136, line_1618, line_total],
#     [r"NACRE", r"Iliadis", r"This Work", r"E$_\alpha=580$ keV", r"E$_\alpha=1136$ keV", r"E$_\alpha=1618$ keV", "Not meas. res."],
#     loc="upper right",
#     bbox_to_anchor=(0.44, 0.54),
#     fontsize=12,
#     frameon=True
# )

# # simple line-only legend for the resonance traces
# line_580 = Line2D([0], [0], color="#009988", lw=1.0)
# line_1136 = Line2D([0], [0], color="#e69f00", lw=1.0)
# line_1618 = Line2D([0], [0], color="#0b77b6", lw=1.0)
# line_total = Line2D([0], [0], color="#BBBBBB", lw=1.0)

# legend2 = ax.legend(
#     [line_580, line_1136, line_1618, line_total],
#     [r"E$_\alpha=580$ keV", r"E$_\alpha=1136$ keV", r"E$_\alpha=1618$ keV", "Not meas. resonances"],
#     loc="upper left",
#     bbox_to_anchor=(0.02, 0.99),
#     fontsize=10,
#     frameon=True
# )

# add the line-only legend now (legend1 is added later by the existing ax.add_artist call)
# ax.add_artist(legend2)


ax.add_artist(legend1)   # keep first legend


# Plot formatting
plt.xscale("log")
ax.set_yscale("log")
# plt.xlim(0.07, 2.0)
ax.set_ylim(0.05, 100.0)
# plt.xlim(0.01, 10.0)
ax.set_xlim(0.05, 1.2)

# keep the small-ts ticks 0.05 and 0.1, then place ticks every 0.5 from 0.5 up to xmax
xticks = [0.05, 0.1] + list(np.arange(0.5, 1.201, 0.5))
ax.set_xticks(xticks)
ax.set_xticklabels([0.05, 0.1, 0.5, 1.0])
# ax.set_xticklabels([r"$5\times10^{-2}$", r"$10^{-1}$"] + [f"{t:.1f}" for t in xticks[2:]])
# ax.set_xticklabels([r"$5\times10^{-2}$", r"$10^{-1}$", r"$5\times10^{-1}$",r"$1\times10^{0}$"])
# protect our labels from the later static call to ax.set_xticklabels([...])
# ax.set_xticks(xticks)
# ax.set_xticklabels([r"$5\times10^{-2}$", r"$10^{-1}$", r"$10^{0}$"])

# set 0.1 interval for x-axis ticks
# ax.set_xticks([0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 1.2])
# ax.set_xticklabels([f"{x:.1f}" for x in [0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 1.2]])
# show ticks every 0.1 but only label a selected subset
# ticks = np.round(np.arange(0.1, 1.21, 0.1), 2)
# ticks = np.round(np.arange(0.1, 1.21, 0.1), 2)
# label_positions = [0.05, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2]
# ax.set_xticks(ticks)
# # ensure 0.05 tick is present and label it
# if not np.any(np.isclose(ticks, 0.05, atol=1e-8)):
#     ticks = np.insert(ticks, 0, 0.05)
#     ax.set_xticks(ticks)

# ax.set_xticklabels([
#     f"{t:.2f}" if np.isclose(t, 0.05, atol=1e-8)
#     else (f"{t:.1f}" if any(np.isclose(t, lp, atol=1e-8) for lp in label_positions) else "")
#     for t in ticks
# ])
# plt.ylim(0.0, 1.2)
# plt.xticks([0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 2.0], [f"{x:.1f}" for x in [0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 2.0]])
#plt.yticks([0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0], [f"{y:.1f}" for y in [0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0]])
# plt.yticks([0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2], [f"{y:.1f}" for y in [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2]])
ax.set_xlabel("Temperature [GK]", fontsize=13)
# Option A: simple offset from the axis using labelpad (points)
ax.set_ylabel("Ratio to This Work", fontsize=13, labelpad=0)

# Option B: explicit axis coordinates (axes fraction units: x,y in [0,1])
# (x < 0 moves the label further left; y controls vertical position)
# ax1.yaxis.set_label_coords(-0.08, 0.5)
# plt.title("Comparison of Reaction Rates", fontsize=16)
ax.tick_params(axis='both', which='major', labelsize=12)
ax.tick_params(axis='both', which='minor', labelsize=12)
# plt.legend(fontsize=13)
# plt.legend(fontsize=14, ncol=1, loc="upper right")

# ax1.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3)
ax.tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3)
#plt.grid(True, alpha=0.3, ls='--')
from matplotlib.patches import Rectangle

ymin, ymax = ax.get_ylim()

ax.add_patch(
    Rectangle(
        (0.1, 0.85*ymax),
        0.3,
        0.08*ymax,
        facecolor='#d55e00',
        edgecolor='none',
        alpha=0.7
    )
)

ax.text(
    0.20,
    0.80*ymax,
    r"He-burning",
    ha='center',
    va='top',
    fontsize=11
)

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

total_all = (
    rates_DC + rates_305 +
    rates_1395 + rates_1530 +
    rates_1768 + rates_2161 + rates_2177 +
    rates_2348 + rates_2350
)

print(total_all[:70])
print(np.min(total_all), np.max(total_all))

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

#plt.figure(figsize=(6,5))
# plt.plot(T9_cont, rates_305, color="C2", label="E$_\\alpha= 305$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_305_iliadis, color="C2", linestyle = "--", label="", linewidth=1.0)
ax.plot(T9_cont, rates_580, color="#CC3311", label=r"$E_{\alpha} = 579\ \mathrm{keV}$", linewidth=1.3, linestyle='--')
#plt.plot(T9_cont_iliadis, rates_580_iliadis, color="C0", linestyle = "--", label="", linewidth=1.0)
ax.plot(T9_cont, rates_1136, color="#CC3311", label=r"$E_{\alpha} = 1136\ \mathrm{keV}$", linewidth=1.3, linestyle='-.') # linestyle=':'
#plt.plot(T9_cont, rates_1530, color="C7", linestyle = "--", label="E$_\\alpha= 1530$ keV", linewidth=1.0)
ax.plot(T9_cont, rates_1618, color="#CC3311", label=r"$E_{\alpha} = 1618\ \mathrm{keV}$", linewidth=1.3, linestyle=':')
# mask = (T9_cont >= 0.1) & (T9_cont <= 1.2)
# ax.plot(T9_cont[mask], total_all[mask], color="#BBBBBB", label="Not meas. res.", linewidth=1.3, linestyle='--')
ax.plot(T9_cont, total_all, color="#BBBBBB", label="non meas. res.", linewidth=1.3, linestyle='--')

#ax.set_ylim(0,2.0)
#ax2.set_ylim(0.08, 10.0)
# set one decimal place for y-axis ticks
#ax.set_yticks(np.arange(0, 2.2, 0.2))
#ax.set_yticklabels([f"{y:.1f}" for y in np.arange(0, 2.1, 0.2)])
# plt.xlabel("Temperature [GK]", fontsize=14); 
# ax.set_ylabel("Fractional contribution", fontsize=13)
ax.set_ylabel("Ratio to This Work", fontsize=13)
# #plt.grid(True, ls="--", lw=0.4)
# ax.legend(fontsize=12, ncol=1, loc="upper right",
#            bbox_to_anchor=(0.45, 0.4))
legend2 = ax.legend(
    fontsize=12,
    ncol=1,
    loc="upper right",
    bbox_to_anchor=(0.67, 0.35)
)
# plt.tight_layout()
# plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3)
ax.tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3, labelsize=12)
# plt.savefig(os.path.join(outdir, "TNRR_combined.pdf"))
# plt.close()
# Create two separate legends and place them at different locations
ax = plt.gca()

# Save updated figure
plt.savefig(os.path.join(outdir, "TNRR_and_FC_260702.pdf"))
plt.close()