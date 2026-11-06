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

plt.rcParams.update({
    "font.family": "serif",
    "mathtext.fontset": "cm",
    "font.size": 12,
})
# --- Load data files ---
import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# ---------------------------------------------------------
# Load Iliadis 2010 rate file
# ---------------------------------------------------------
iliadis = pd.read_csv(
    "RatesIliadis2025/RatesMC.out",
    delim_whitespace=True,
    comment="#",
    header=None
)

T9_iliadis = iliadis.iloc[:, 0].values
rate_low_iliadis = iliadis.iloc[:, 1].values
rate_med_iliadis = iliadis.iloc[:, 2].values
rate_hi_iliadis  = iliadis.iloc[:, 3].values


# ---------------------------------------------------------
# Load This Work rate file
# ---------------------------------------------------------
this = pd.read_csv(
    "RatesAnup260715/RatesMC.out",
    delim_whitespace=True,
    comment="#",
    header=None
)

T9 = this.iloc[:, 0].values
rate_low_this = this.iloc[:, 1].values
rate_med_this = this.iloc[:, 2].values
rate_hi_this  = this.iloc[:, 3].values


# ---------------------------------------------------------
# Load NACRE (Gorres)
# ---------------------------------------------------------
Gor = pd.read_csv(
    "Gorres200.txt",
    delim_whitespace=True,
    comment="#",
    header=None
)

T9_NACRE = Gor.iloc[:, 0].values
rate_med_NACRE = Gor.iloc[:, 5].values
rate_low_NACRE = Gor.iloc[:, 6].values
rate_hi_NACRE  = Gor.iloc[:, 7].values

# ---------------------------------------------------------
# Output directory
# ---------------------------------------------------------
outdir = "RatesMC_plots_260715"
os.makedirs(outdir, exist_ok=True)

plt.rcParams.update({'font.size': 10})


# =========================================================
# Iliadis / This Work
# =========================================================
T9_ratio_iliadis = T9

ratio_iliadis_to_this = rate_med_iliadis / rate_med_this

ratio_lo_iliadis_to_this = (
    rate_low_iliadis /
    rate_med_this
)

ratio_hi_iliadis_to_this = (
    rate_hi_iliadis /
    rate_med_this
)


# =========================================================
# Find matching temperatures with NACRE
# =========================================================
def find_matching_indices(arr1, arr2, tol=1e-6):

    idx1 = []
    idx2 = []

    for i, val in enumerate(arr1):

        diff = np.abs(arr2 - val)
        j = np.argmin(diff)

        if diff[j] < tol:
            idx1.append(i)
            idx2.append(j)

    return np.array(idx1), np.array(idx2)


idx_this_nacre, idx_nacre = find_matching_indices(
    T9,
    T9_NACRE
)

if len(idx_this_nacre) == 0:
    raise RuntimeError("No common temperatures found with NACRE.")


T9_ratio_nacre = T9[idx_this_nacre]


# =========================================================
# NACRE / This Work
# =========================================================
ratio_NACRE_to_this = (
    rate_med_NACRE[idx_nacre] /
    rate_med_this[idx_this_nacre]
)

ratio_lo_NACRE_to_this = (
    rate_low_NACRE[idx_nacre] /
    rate_med_this[idx_this_nacre]
)

ratio_hi_NACRE_to_this = (
    rate_hi_NACRE[idx_nacre] /
    rate_med_this[idx_this_nacre]
)


# =========================================================
# This Work / This Work
# =========================================================
ratio_this_to_this = np.ones_like(T9)

ratio_lo_this_to_this = (
    rate_low_this /
    rate_med_this
)

ratio_hi_this_to_this = (
    rate_hi_this /
    rate_med_this
)


# =========================================================
# Ratio Plot
# =========================================================
plt.figure(figsize=(3, 6))
fig, ax = plt.subplots()


# -----------------------------
# NACRE / THIS
# -----------------------------
ax.plot(
    T9_ratio_nacre,
    ratio_NACRE_to_this,
    color="black",
    lw=1.0
)

ax.fill_between(
    T9_ratio_nacre,
    ratio_lo_NACRE_to_this,
    ratio_hi_NACRE_to_this,
    color="black",
    alpha=0.30,
    edgecolor="none"
)


# -----------------------------
# Iliadis / THIS
# -----------------------------
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
    edgecolor="none"
)


# -----------------------------
# THIS / THIS
# -----------------------------
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
    edgecolor="none"
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
    [r"$\mathrm{NACRE}$", r"$\mathrm{Iliadis}$", r"$\mathrm{This\ Work}$"],
    loc="upper right",
    bbox_to_anchor=(0.98, 0.99),  # exact position in axis coords (x, y)
    fontsize=12,
    frameon=True
)

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

# plt.ylim(0.0, 1.2)
# plt.xticks([0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 2.0], [f"{x:.1f}" for x in [0.1, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 2.0]])
#plt.yticks([0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0], [f"{y:.1f}" for y in [0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0]])
# plt.yticks([0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2], [f"{y:.1f}" for y in [0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2]])
ax.set_xlabel(r"$\mathrm{Temperature\ [GK]}$", fontsize=14, labelpad=-1) 
# Option A: simple offset from the axis using labelpad (points)
ax.set_ylabel(r"$\mathrm{Ratio\ to\ This\ Work}$", fontsize=13, labelpad=-1)

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
    r"$\mathrm{He-burning}$",
    ha='center',
    va='top',
    fontsize=12
)

# # ---------- 2) Fraction of each resonance: this work vs literature (dashed = lit) ----------#
# # Load contribution files
this_cont = pd.read_csv("RatesAnup260715/RatesMC.cont", delim_whitespace=True, comment="#", header=None)
T9_cont = this_cont.iloc[:, 0].values
rates_DC = this_cont.iloc[:,2].values
rates_580 = this_cont.iloc[:,8].values
rates_1136 = this_cont.iloc[:,56].values
rates_1395 = this_cont.iloc[:,14].values
rates_1530 = this_cont.iloc[:,17].values
rates_1618 = this_cont.iloc[:,20].values
rates_1768 = this_cont.iloc[:,23].values
rates_2161 = this_cont.iloc[:,26].values
rates_2177 = this_cont.iloc[:,29].values
rates_2348 = this_cont.iloc[:,32].values
rates_2350 = this_cont.iloc[:,35].values
rates_305 = this_cont.iloc[:,53].values

total_all = (
    rates_DC + rates_305 +
    rates_1395 + rates_1530 +
    rates_1768 + rates_2161 + rates_2177 +
    rates_2348 + rates_2350
)

# print(total_all[:70])
# print(np.min(total_all), np.max(total_all))

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
ax.plot(T9_cont, total_all, color="#BBBBBB", label=r"$\mathrm{Not\ studied\ here}$", linewidth=1.3, linestyle='--')

#ax.set_ylim(0,2.0)
#ax2.set_ylim(0.08, 10.0)
# set one decimal place for y-axis ticks
#ax.set_yticks(np.arange(0, 2.2, 0.2))
#ax.set_yticklabels([f"{y:.1f}" for y in np.arange(0, 2.1, 0.2)])
# plt.xlabel("Temperature [GK]", fontsize=14); 
# ax.set_ylabel("Fractional contribution", fontsize=13)
ax.set_ylabel(r"$\mathrm{Ratio\ to\ This\ Work}$", fontsize=14, labelpad=-6)
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
plt.savefig(os.path.join(outdir, "TNRR_and_FC_260715.pdf"))
plt.close()