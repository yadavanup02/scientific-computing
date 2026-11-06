#!/usr/bin/env python3
"""
Produces:
 1) Fraction of each resonance (this work, with bands optional)
 2) Fraction of each resonance: this work vs literature (dashed = lit)
 3) Contribution of each resonance with 1σ band and total with band
 4) Ratio (this / literature) with conservative 1σ band

Band construction: analytic ±1σ per-resonance:
  - maximize: (wg + dwg) and (Er - dEr)  (gives upper total)
  - minimize: (wg - dwg) and (Er + dEr)  (gives lower total)
This mimics the PRL Fig.2 approach.
"""

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd
import os

# --- Load Iliadis 2010 rate file ---
iliadis = pd.read_csv("Iliadis2010_Rate_n14ag_Paper.txt", delim_whitespace=True, comment="#", header=None)
T9_iliadis = iliadis.iloc[:, 0].values
rate_med_iliadis = iliadis.iloc[:, 2].values
rate_lo_iliadis  = iliadis.iloc[:, 1].values
rate_hi_iliadis  = iliadis.iloc[:, 3].values

NACR = pd.read_csv("nacr.txt", delim_whitespace=True, comment="#", header=None)
T9_NACR = NACR.iloc[:, 0].values
rate_med_NACR = NACR.iloc[:, 1].values

Gor = pd.read_csv("Gorres200.txt", delim_whitespace=True, comment="#", header=0)
T9_Gor = Gor.iloc[:, 0].values
rate_med_Gor = Gor.iloc[:, 3].values

# ---------- Inputs ----------
# Literature (lab alpha energies in keV, wg in eV) and their 1σ uncertainties
Er_lab_keV_lit = np.array([573.0, 1136.0, 1395.0, 1532.0, 1618.0])
wg_eV_lit      = np.array([4.5e-5, 2.1e-2, 9.0e-3, 1.6, 0.45])
dEr_keV_lit    = np.array([3.0,   3.0,   3.0,  2.0,  2.0])     # keV
dwg_eV_lit     = np.array([0.3e-5,0.3e-3,1.0e-3,0.13,0.05])    # eV

# Your measured values (0 means not measured -> use literature)
measured_Er_lab_keV = np.array([580.3, 1136.9,   0.0, 0.0, 1618.1])
measured_dEr_keV    = np.array([0.2,   0.2,      0.0, 0.0,  0.1])
measured_wg_eV      = np.array([4.6e-5, 2.35e-2, 0.0, 0.0, 0.455])
measured_dwg_eV     = np.array([0.3e-5, 0.6e-3,  0.0, 0.0, 0.007])

# ---------- grid & constants ----------
T9 = np.round(np.arange(0.01, 10.00 + 0.01, 0.01), 2)
pref = 1.5394e11
m_alpha = 4.002602; m_14N = 14.003074
mu = (m_alpha * m_14N) / (m_alpha + m_14N)
conv = m_14N / (m_14N + m_alpha)   # lab alpha -> CM

tiny = 1e-300
outdir = "plots_all"
os.makedirs(outdir, exist_ok=True)

# ---------- Build "this work" arrays (use measured when non-zero) ----------
Er_lab_keV_this = Er_lab_keV_lit.copy()
dEr_keV_this = dEr_keV_lit.copy()
wg_eV_this = wg_eV_lit.copy()
dwg_eV_this = dwg_eV_lit.copy()

mask_meas = (measured_wg_eV > 0) | (measured_Er_lab_keV > 0)
for i in range(len(Er_lab_keV_lit)):
    if mask_meas[i]:
        # prefer measured non-zero entries; otherwise fallback to lit
        if measured_Er_lab_keV[i] > 0:
            Er_lab_keV_this[i] = measured_Er_lab_keV[i]
        if measured_dEr_keV[i] > 0:
            dEr_keV_this[i] = measured_dEr_keV[i]
        if measured_wg_eV[i] > 0:
            wg_eV_this[i] = measured_wg_eV[i]
        if measured_dwg_eV[i] > 0:
            dwg_eV_this[i] = measured_dwg_eV[i]

# Convert to MeV, CM energies
Er_cm_MeV_lit = (Er_lab_keV_lit * 1e-3) * conv
Er_cm_MeV_this = (Er_lab_keV_this * 1e-3) * conv
dEr_cm_MeV_lit = (dEr_keV_lit * 1e-3) * conv
dEr_cm_MeV_this = (dEr_keV_this * 1e-3) * conv
wg_MeV_lit = wg_eV_lit * 1e-6
wg_MeV_this = wg_eV_this * 1e-6
dwg_MeV_lit = dwg_eV_lit * 1e-6
dwg_MeV_this = dwg_eV_this * 1e-6

nres = len(Er_lab_keV_lit)

# ---------- kernel function (per-resonance temperature dependence) ----------
def kernel(Er_cm):
    # Er_cm : array shape (nres,)
    # returns (nres, nT)
    return (1.0 / (mu * T9))[None, :]**1.5 * np.exp(-11.605 * Er_cm[:, None] / T9[None, :])

# ---------- nominal rates ----------
K_lit = kernel(Er_cm_MeV_lit)   # (nres, nT)
K_this = kernel(Er_cm_MeV_this)
rate_nom_lit = pref * np.sum(wg_MeV_lit[:, None] * K_lit, axis=0)
rate_nom_this = pref * np.sum(wg_MeV_this[:, None] * K_this, axis=0)

# ---------- analytic conservative ±1σ bands ----------
# maximize by (wg+dwg) & (Er - dEr). minimize by (wg-dwg) & (Er + dEr)
wg_hi_lit = wg_MeV_lit + dwg_MeV_lit
wg_lo_lit = np.clip(wg_MeV_lit - dwg_MeV_lit, 0.0, None)
Er_hi_lit = Er_cm_MeV_lit + dEr_cm_MeV_lit
Er_lo_lit = Er_cm_MeV_lit - dEr_cm_MeV_lit

wg_hi_this = wg_MeV_this + dwg_MeV_this
wg_lo_this = np.clip(wg_MeV_this - dwg_MeV_this, 0.0, None)
Er_hi_this = Er_cm_MeV_this + dEr_cm_MeV_this
Er_lo_this = Er_cm_MeV_this - dEr_cm_MeV_this

rate_hi_lit  = pref * np.sum(wg_hi_lit[:, None] * kernel(Er_lo_lit), axis=0)
rate_lo_lit  = pref * np.sum(wg_lo_lit[:, None] * kernel(Er_hi_lit), axis=0)
rate_hi_this = pref * np.sum(wg_hi_this[:, None] * kernel(Er_lo_this), axis=0)
rate_lo_this = pref * np.sum(wg_lo_this[:, None] * kernel(Er_hi_this), axis=0)

# floor for plotting
rate_lo_lit  = np.maximum(rate_lo_lit, tiny)
rate_hi_lit  = np.maximum(rate_hi_lit, tiny)
rate_lo_this = np.maximum(rate_lo_this, tiny)
rate_hi_this = np.maximum(rate_hi_this, tiny)

# ---------- per-resonance nominal & band contributions ----------
# contribution per resonance = pref * wg * kernel(Er_cm)
contrib_nom_lit = pref * (wg_MeV_lit[:, None] * K_lit)   # (nres, nT)
contrib_nom_this = pref * (wg_MeV_this[:, None] * K_this)

contrib_hi_lit = pref * (wg_hi_lit[:, None] * kernel(Er_lo_lit))
contrib_lo_lit = pref * (wg_lo_lit[:, None] * kernel(Er_hi_lit))

contrib_hi_this = pref * (wg_hi_this[:, None] * kernel(Er_lo_this))
contrib_lo_this = pref * (wg_lo_this[:, None] * kernel(Er_hi_this))

# floor
contrib_lo_lit = np.maximum(contrib_lo_lit, tiny)
contrib_hi_lit = np.maximum(contrib_hi_lit, tiny)
contrib_lo_this = np.maximum(contrib_lo_this, tiny)
contrib_hi_this = np.maximum(contrib_hi_this, tiny)

# increase the fontsize in all plots?
plt.rcParams.update({'font.size': 12})

# Margins are best adjusted after creating each figure, or use tight_layout for automatic adjustment.
# Remove the global plt.subplots_adjust call here.


# ---------- 1) Fraction of each resonance (this work nominal) ----------
frac_this_nom = contrib_nom_this / np.clip(np.sum(contrib_nom_this, axis=0), a_min=tiny, a_max=None)
plt.figure(figsize=(8,5))
for i in range(nres):
    plt.plot(T9, frac_this_nom[i], label=f"E$_\\alpha= {int(Er_lab_keV_this[i])}$ keV")
plt.xscale("log"); plt.xlim(0.1, 10)
plt.ylim(0, 1.05)
plt.xlabel("Temperature [GK]"); plt.ylabel("Fractional contribution (this work)")
plt.grid(True, ls="--", lw=0.4); plt.legend(fontsize="small", ncol=1)
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.tight_layout()
plt.savefig(os.path.join(outdir, "1_fraction_each_resonance_this_nominal.pdf"))
plt.close()

# ---------- 2) Fraction of each resonance: this work vs literature (dashed = lit) ----------
total_nom_lit = np.sum(contrib_nom_lit, axis=0)
total_nom_this = np.sum(contrib_nom_this, axis=0)
frac_lit_nom = contrib_nom_lit / np.clip(total_nom_lit, a_min=tiny, a_max=None)
frac_this_nom = contrib_nom_this / np.clip(total_nom_this, a_min=tiny, a_max=None)

plt.figure(figsize=(8,5))
for i in range(nres):
    plt.plot(T9, frac_this_nom[i], color=f"C{i}", label=f"this E$_\\alpha= {int(Er_lab_keV_this[i])}$ keV", linewidth=1.0)
    plt.plot(T9, frac_lit_nom[i], color=f"C{i}", linestyle="--", label=f"lit E$_\\alpha= {int(Er_lab_keV_lit[i])}$ keV", alpha=0.6, linewidth=1.0)
plt.xscale("log"); plt.xlim(0.1, 10); plt.ylim(0,1.05)
plt.xlabel("Temperature [GK]"); plt.ylabel("Fractional contribution")
plt.grid(True, ls="--", lw=0.4)
plt.legend(fontsize="small", ncol=1)
plt.tight_layout()
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.savefig(os.path.join(outdir, "2_fraction_this_vs_lit.pdf"))
plt.close()

# ---------- 3) Contribution of each resonance with band and total with band ----------
plt.figure(figsize=(10,7))
colors = plt.cm.tab10(np.arange(nres))
for i in range(nres):
    # band for this work per resonance
    valid_i = np.isfinite(contrib_lo_this[i]) & np.isfinite(contrib_hi_this[i]) & (contrib_hi_this[i] > 0)
    plt.fill_between(T9, contrib_lo_this[i], contrib_hi_this[i], where=valid_i, color=colors[i], alpha=0.35)
    plt.plot(T9, contrib_nom_this[i], color=colors[i], linewidth=0.5, label=f"E$_\\alpha= {int(Er_lab_keV_this[i])}$ keV")
# total band & nominal (this work)
plt.fill_between(T9, rate_lo_this, rate_hi_this, color='gray', alpha=0.35, label="") #"total ±1σ (this)"
plt.plot(T9, rate_nom_this, color='k', linewidth=0.5, label="total (this)")
plt.xscale("log"); plt.yscale("log"); plt.xlim(0.1, 10); plt.ylim(1e-15, 1e3)
plt.xlabel("Temperature [GK]")
#plt.ylabel(r"Contribution (cm$^3$ mol$^{-1}$ s$^{-1}$)")
plt.ylabel(r"$N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
plt.grid(True, ls="--", lw=0.4); plt.legend(fontsize="small", ncol=1)
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.tight_layout()
plt.savefig(os.path.join(outdir, "3_contributions_each_with_bands_and_total.pdf"))
plt.close()
## calculation of ratio plot
# Find indices where both T9 and T9_iliadis have matching values
common_T9 = np.intersect1d(T9, T9_iliadis)
# Get indices for T9 and T9_iliadis
idx_this = np.nonzero(np.isin(T9, common_T9))[0]
idx_iliadis = np.nonzero(np.isin(T9_iliadis, common_T9))[0]
# Compute ratio only where both are available
ratio = rate_nom_this[idx_this] / rate_med_iliadis[idx_iliadis]
T9_ratio = T9[idx_this]

# ---------- 4) Ratio: this / literature with conservative band ----------
# Conservative ratio bounds: hi = this_hi / lit_lo , lo = this_lo / lit_hi
ratio_nom = rate_nom_this / np.clip(rate_nom_lit, tiny, None)
ratio_hi = (rate_hi_this) / np.clip(rate_lo_lit, tiny, None)
ratio_lo = (rate_lo_this) / np.clip(rate_hi_lit, tiny, None)
ratio_lo = np.maximum(ratio_lo, tiny)
ratio_lit = rate_nom_lit / np.clip(rate_nom_lit, tiny, None)  # horizontal line at 1

plt.figure(figsize=(8,6))
plt.plot(T9, ratio_nom, color="orange", label=r"$\langle\sigma v\rangle _{this}$ / $\langle\sigma v\rangle _{lit}$")
#plt.fill_between(T9, ratio_lo, ratio_hi, color='lightskyblue', alpha=0.35, label=" ± band")
plt.plot(T9, ratio_lit, color="k") #, linestyle="--", label="Literature")
plt.plot(T9_ratio, ratio, color="lightskyblue", linestyle="--", label=r"$\langle\sigma v\rangle _{this}$ / $\langle\sigma v\rangle _{Iliadis}$")
plt.xscale("log") 
plt.xlim(0.1,10)
# set reasonable y-limits: include 1 and the band extent
ymax = np.nanpercentile(np.clip(ratio_hi, a_min=0, a_max=None), 99)
plt.ylim(0, max(1.2, ymax*1.05))
plt.xlabel("Temperature [GK]"); plt.ylabel("Ratio of reaction rates")
plt.grid(True, ls="--", lw=0.4); plt.legend()
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.tight_layout()  
plt.savefig(os.path.join(outdir, "4_rates_ratio_v2.pdf"))
plt.close()



# --- Plot comparison ---
plt.figure(figsize=(8,5))
plt.plot(T9, rate_nom_this, color="tab:orange", lw=0.5, label="This work")
plt.fill_between(T9, rate_lo_this, rate_hi_this, color="tab:orange", alpha=0.3)
# Plot Iliadis 2010 median rate and its band
plt.plot(T9_iliadis, rate_med_iliadis, color="tab:blue", lw=0.5, label="Iliadis 2010")
plt.fill_between(T9_iliadis, rate_lo_iliadis, rate_hi_iliadis, color="tab:blue", alpha=0.3)
# Plot NACR median rate
plt.plot(T9_NACR, rate_med_NACR, color="tab:green", lw=0.5, label="NACR")
#plt.fill_between(T9_NACR, rate_med_NACR*0.8, rate_med_NACR*1.2, color="tab:green", alpha=0.3)

# Plot örres 2000 median rate
plt.plot(T9_Gor, rate_med_Gor, color="tab:red", lw=0.5, label="Görres 2000")
#plt.fill_between(T9_Gor, rate_med_Gor*0.8, rate_med_Gor*1.2, color="tab:red", alpha=0.3)

plt.xscale("log"); plt.yscale("log"); plt.xlim(0.1,1.0); plt.ylim(1e-14, 1e0)
plt.xlabel("T$_9$ (GK)"); plt.ylabel(r"$N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
plt.legend()
plt.grid(True, which="both", ls="--", lw=0.4)
plt.tight_layout()
plt.savefig("plots_all/compare_total_rate_with_iliadis_v1.pdf")
plt.show()

print("Saved plots to", outdir)

# line_iliadis, = plt.plot(T9_iliadis, rate_med_iliadis, color="tab:blue", lw=0.5)
# band_iliadis = plt.fill_between(T9_iliadis, rate_lo_iliadis, rate_hi_iliadis, color="tab:blue", alpha=0.3)

# # Custom legend: show line and band together for Iliadis 2010
# custom_legend = [
#     Line2D([0], [0], color="tab:orange", lw=0.5, label="This work"),
#     Patch(facecolor="tab:orange", edgecolor="none", alpha=0.3, label="This work ±1σ"),
#     Line2D([0], [0], color="tab:blue", lw=0.5, label="Iliadis 2010 (median)"),
#     Patch(facecolor="tab:blue", edgecolor="none", alpha=0.3, label="Iliadis 2010 ±1σ"),
# ]
# plt.legend(handles=custom_legend)

plt.figure(figsize=(8,4))
plt.plot(T9_ratio, ratio, color="k")
plt.xscale("log"); plt.xlim(0.1,10)
plt.xlabel("T$_9$ (GK)"); plt.ylabel("This work / Iliadis 2010")
plt.grid(True, ls="--", lw=0.4)
plt.tight_layout()
plt.savefig("plots_all/ratio_this_to_iliadis.pdf")
plt.show()
