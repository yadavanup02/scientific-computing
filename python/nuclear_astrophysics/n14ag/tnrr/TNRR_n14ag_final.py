#!/usr/bin/env python3
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from matplotlib.lines import Line2D
import pandas as pd
import os
from scipy.interpolate import make_interp_spline

# --- Load Iliadis 2010 rate file ---
# iliadis = pd.read_csv("RateIliadis/RatesMC.out", delim_whitespace=True, comment="#", header=None)  #sep="\t"
iliadis = pd.read_csv("RatesIliadis_final/RatesMC.out", delim_whitespace=True, comment="#", header=None)  #sep="\t"
T9_iliadis = iliadis.iloc[:, 0].values
rate_low_iliadis  = iliadis.iloc[:, 1].values
rate_med_iliadis = iliadis.iloc[:, 2].values
rate_hi_iliadis  = iliadis.iloc[:, 3].values
# --- Load this work rate file ---
#this = pd.read_csv("RateAnup/RatesMC.out", delim_whitespace=True, comment="#", header=None)
this = pd.read_csv("RatesAnup_final/RatesMC.out", delim_whitespace=True, comment="#", header=None)
T9 = this.iloc[:, 0].values
rate_low_this = this.iloc[:, 1].values
rate_med_this = this.iloc[:, 2].values
rate_hi_this = this.iloc[:, 3].values

NACR = pd.read_csv("nacr.txt", delim_whitespace=True, comment="#", header=None)
T9_NACR = NACR.iloc[:, 0].values
rate_med_NACR = NACR.iloc[:, 1].values

Gor = pd.read_csv("Gorres200.txt", delim_whitespace=True, comment="#", header=0)
T9_Gor = Gor.iloc[:, 0].values
T9_NACRE = Gor.iloc[:, 0].values
rate_med_Gor = Gor.iloc[:, 3].values
rate_med_NACRE = Gor.iloc[:, 5].values
# First, let's print some diagnostic information
print("\n=== Data size diagnostics ===")
print(f"Your work: T9 size = {len(T9)}, rate size = {len(rate_med_this)}")
print(f"Iliadis: T9 size = {len(T9_iliadis)}, rate size = {len(rate_med_iliadis)}")
print(f"NACR (from nacr.txt): T9 size = {len(T9_NACR)}, rate size = {len(rate_med_NACR)}")
print(f"NACRE (from Gorres200.txt): T9 size = {len(T9_NACRE)}, rate size = {len(rate_med_NACRE)}")
# --- Output directory for plots ---
outdir = "RatesMC_plots_final"
os.makedirs(outdir, exist_ok=True)

# increase the fontsize in all plots?
plt.rcParams.update({'font.size': 10})
#---- Ratio plot: this / Iliadis with conservative band ----
tiny = 1e-300
ratio_this_to_iliadis = rate_med_this / np.clip(rate_med_iliadis, tiny, None)

ratio_illiadis = rate_med_iliadis / np.clip(rate_med_iliadis, tiny, None)  # horizontal line at 1

# # Find common temperature values
# common_T9_1 = np.intersect1d(T9, T9_NACRE)

# # Get indices for BOTH arrays
# idx_this = np.nonzero(np.isin(T9, common_T9_1))[0]      # Indices in your data
# idx_nacre = np.nonzero(np.isin(T9_NACRE, common_T9_1))[0]  # Indices in NACRE data

# # Compute ratio using matching indices
# ratio_this_to_NACRE = rate_med_this[idx_this] / rate_med_NACRE[idx_nacre]
# T9_ratio_1 = T9[idx_this]  # or T9_NACRE[idx_nacre] - they should be the same values


common_T9 = np.intersect1d(T9_Gor, T9_iliadis)
idx_iliadis = np.nonzero(np.isin(T9_iliadis, common_T9))[0]
T9_ratio = T9_iliadis[idx_iliadis]
plt.figure(figsize=(8,5))
#plt.plot(T9_iliadis, ratio_illiadis, color="lightskyblue", label=r"$\langle\sigma v\rangle _{this}$ / $\langle\sigma v\rangle _{lit}$")
#plt.fill_between(T9, ratio_lo, ratio_hi, color='lightskyblue', alpha=0.35, label=" ± band")
#plt.plot(T9, rate_med_this, color="k") #, linestyle="--", label="Literature")
plt.plot(T9, ratio_this_to_iliadis, color="blue", label=r"$\frac{\langle\sigma v\rangle_{\mathrm{this}}}{\langle\sigma v\rangle_{\mathrm{Iliadis}}}$")
#plt.plot(T9_ratio, ratio_this_to_NACR, color="lightskyblue", label=r"$\langle\sigma v\rangle _{this}$ / $\langle\sigma v\rangle _{NACRE}$")
#plt.plot(T9_ratio_1, ratio_this_to_NACRE, color="red", label=r"$\langle\sigma v\rangle _{this}$ / $\langle\sigma v\rangle _{NACRE}$")
#---- shaded area for helium burning temperature 0.1 to 0.5 GK ----
plt.fill_betweenx(y=[0, 1.05], x1=0.1, x2=0.4, color='gray', alpha=0.2)
#plt.xscale("log") 
plt.xlim(0.07,1.0)
plt.xscale("log")
plt.ylim(0.4, 1.05)
plt.xlabel("Temperature [GK]"); plt.ylabel("Ratio of reaction rates")
#plt.grid(True, ls="--", lw=0.4)
plt.legend(fontsize="large")
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.tight_layout()
plt.savefig(os.path.join(outdir, "Ratio_this_to_iliadis_NACRE_v3.pdf"))
plt.close()

# --- Plot comparison ---
plt.figure(figsize=(8,5))
plt.plot(T9, rate_med_this, color="tab:orange", lw=0.5, label="This work")
#plt.fill_between(T9, rate_low_this, rate_hi_this, color="tab:orange", alpha=0.3)
# Plot Iliadis 2010 median rate and its band
plt.plot(T9_iliadis, rate_med_iliadis, color="tab:blue", lw=0.5, label="Iliadis 2010") 
#plt.fill_between(T9_iliadis, rate_low_iliadis, rate_hi_iliadis, color="tab:blue", alpha=0.3)
# Plot NACR median rate
plt.plot(T9_Gor, rate_med_NACRE, color="tab:green", lw=0.5, label="NACRE 1999")
#plt.fill_between(T9_NACR, rate_med_NACR*0.8, rate_med_NACR*1.2, color="tab:green", alpha=0.3)
# Plot örres 2000 median rate
#plt.plot(T9_Gor, rate_med_Gor, color="tab:red", lw=0.5, label="Görres 2000")
#plt.fill_between(T9_Gor, rate_med_Gor*0.8, rate_med_Gor*1.2, color="tab:red", alpha=0.3)
#---- shaded area for helium burning temperature 0.1 to 0.5 GK ----
plt.fill_betweenx(y=[0, 1.05], x1=0.1, x2=0.4, color='gray', alpha=0.2)
#plt.xscale("log");
plt.yscale("log"); plt.xscale("log")
plt.xlim(0.06, 1.0)
plt.xticks(np.arange(0.1, 1.0, 0.1), [f"{x:.1f}" for x in np.arange(0.1, 1.0, 0.1)])
plt.minorticks_on()
plt.tick_params(axis='both', which='minor', labelsize=10)
plt.ylim(1e-26, 1e-2)
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.xlabel("Temperature [GK]"); plt.ylabel(r"$N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
plt.legend()
#plt.grid(True, which="both", ls="--", lw=0.4)
plt.tight_layout()
plt.savefig(os.path.join(outdir,"TNRR_comparison_v2.pdf"))
plt.show()

# # --- Contribution of each resdonance ---
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



# # ---------- 1) Fraction of each resonance (this work nominal) ----------
# frac_this_nom = contrib_nom_this / np.clip(np.sum(contrib_nom_this, axis=0), a_min=tiny, a_max=None)
plt.figure(figsize=(8,5))
plt.plot(T9_cont, rates_DC, label="Direct capture", linewidth=1.5)
plt.plot(T9_cont, rates_305, label="E$_\\alpha= 305$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_580, label="E$_\\alpha= 580$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_1136, label="E$_\\alpha= 1136$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_1395, label="E$_\\alpha= 1395$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_1530, label="E$_\\alpha= 1530$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_1618, label="E$_\\alpha= 1618$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_1768, label="E$_\\alpha= 1768$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_2161, label="E$_\\alpha= 2161$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_2177, label="E$_\\alpha= 2177$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_2348, label="E$_\\alpha= 2348$ keV", linewidth=1.5)
plt.plot(T9_cont, rates_2350, label="E$_\\alpha= 2350$ keV", linewidth=1.5)

#---- shaded area for helium burning temperature 0.1 to 0.5 GK ----
plt.fill_betweenx(y=[0, 1.05], x1=0.1, x2=0.4, color='gray', alpha=0.2)


plt.xscale("log"); plt.xlim(0.01, 10)
plt.ylim(0, 1.05)
plt.xlabel("Temperature [GK]"); plt.ylabel("Fractional contribution")
#plt.grid(True, ls="--", lw=0.4)
plt.legend(fontsize="small", ncol=1)
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.tight_layout()
plt.savefig(os.path.join(outdir, "TNRR_cont_v2.pdf"))
plt.close()
# for i in range(nres):
#     plt.plot(T9, frac_this_nom[i], label=f"E$_\\alpha= {int(Er_lab_keV_this[i])}$ keV")


# # ---------- 2) Fraction of each resonance: this work vs literature (dashed = lit) ----------
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

plt.figure(figsize=(6,5))
plt.plot(T9_cont, rates_DC, color="k", label="Direct capture", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_DC_iliadis, color="k", linestyle = "--", linewidth=1.0)
plt.plot(T9_cont, rates_305, color="C2", label="E$_\\alpha= 305$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_305_iliadis, color="C2", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_580, color="C3", label="E$_\\alpha= 580$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_580_iliadis, color="C0", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_1136, color="C0", label="E$_\\alpha= 1136$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_1136_iliadis, color="C1", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_1395, color="C2", label="E$_\\alpha= 1395$ keV", linewidth=1.0)
# plt.plot(T9_cont_iliadis, rates_1395_iliadis, color="C2", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_1530, color="C1", label="E$_\\alpha= 1530$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_1530_iliadis, color="C3", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_1618, color="C4", label="E$_\\alpha= 1618$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_1618_iliadis, color="C4", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_1768, color="C5", label="E$_\\alpha= 1768$ keV", linewidth=1.0)
# plt.plot(T9_cont_iliadis, rates_1768_iliadis, color="C5", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_2161, color="C6", label="E$_\\alpha= 2161$ keV", linewidth=1.0)
# plt.plot(T9_cont_iliadis, rates_2161_iliadis, color="C6", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_2177, color="C7", label="E$_\\alpha= 2177$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_2177_iliadis, color="C7", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_2348, color="C8", label="E$_\\alpha= 2348$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_2348_iliadis, color="C8", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_2350, color="C9", label="E$_\\alpha= 2350$ keV", linewidth=1.0)
#plt.plot(T9_cont_iliadis, rates_2350_iliadis, color="C9", linestyle = "--", label="", linewidth=1.0)
#---- shaded area for helium burning temperature 0.1 to 0.5 GK ----
# plt.fill_betweenx(y=[0, 1.05], x1=0.1, x2=0.4, color='gray', alpha=0.2)
# write (a) on the top right corner
plt.text(0.15, 0.90, "(a)", transform=plt.gca().transAxes, fontsize=16, verticalalignment='top', horizontalalignment='right')   
plt.tick_params(axis='both', which='major', labelsize=13)
plt.tick_params(axis='both', which='minor', labelsize=13)
plt.legend(fontsize=12)
# plt.tick_params(axis='x', which='both', labelsize=142)
# plt.legend(fontsize=14, prop={'style': 'italic'})
plt.xscale("log"); plt.xlim(0.01, 10); plt.ylim(0,1.05)
plt.xlabel("Temperature [GK]", fontsize=14); plt.ylabel("Fractional contribution", fontsize=14)
#plt.grid(True, ls="--", lw=0.4)
plt.legend(fontsize=12, ncol=1, loc="upper right")
plt.tight_layout()
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True, length=3)
plt.savefig(os.path.join(outdir, "TNRR_cont_v3.pdf"))
plt.close()

plt.figure(figsize=(6,5))
plt.plot(T9_cont, rates_DC, color="k", label="Direct capture", linewidth=1.0)
plt.plot(T9_cont_iliadis, rates_DC_iliadis, color="k", linestyle = "--", linewidth=1.0)
plt.plot(T9_cont, rates_305, color="C2", label="E$_\\alpha= 305$ keV", linewidth=1.0)
plt.plot(T9_cont_iliadis, rates_305_iliadis, color="C2", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_580, color="C3", label="E$_\\alpha= 579$ keV", linewidth=1.0)
plt.plot(T9_cont_iliadis, rates_580_iliadis, color="C3", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_1136, color="C0", label="E$_\\alpha= 1136$ keV", linewidth=1.0)
plt.plot(T9_cont_iliadis, rates_1136_iliadis, color="C0", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_1395, color="C2", label="E$_\\alpha= 1395$ keV", linewidth=1.0)
# plt.plot(T9_cont_iliadis, rates_1395_iliadis, color="C2", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_1530, color="C1", label="E$_\\alpha= 1530$ keV", linewidth=1.0)
plt.plot(T9_cont_iliadis, rates_1530_iliadis, color="C1", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_1618, color="C4", label="E$_\\alpha= 1618$ keV", linewidth=1.0)
plt.plot(T9_cont_iliadis, rates_1618_iliadis, color="C4", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_1768, color="C5", label="E$_\\alpha= 1768$ keV", linewidth=1.0)
# plt.plot(T9_cont_iliadis, rates_1768_iliadis, color="C5", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_2161, color="C6", label="E$_\\alpha= 2161$ keV", linewidth=1.0)
# plt.plot(T9_cont_iliadis, rates_2161_iliadis, color="C6", linestyle = "--", label="", linewidth=1.0)
plt.plot(T9_cont, rates_2177, color="C7", label="E$_\\alpha= 2177$ keV", linewidth=1.0)
plt.plot(T9_cont_iliadis, rates_2177_iliadis, color="C7", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_2348, color="C8", label="E$_\\alpha= 2348$ keV", linewidth=1.0)
# plt.plot(T9_cont_iliadis, rates_2348_iliadis, color="C8", linestyle = "--", label="", linewidth=1.0)
# plt.plot(T9_cont, rates_2350, color="C9", label="E$_\\alpha= 2350$ keV", linewidth=1.0)
# plt.plot(T9_cont_iliadis, rates_2350_iliadis, color="C9", linestyle = "--", label="", linewidth=1.0)
#---- shaded area for helium burning temperature 0.1 to 0.5 GK ----
plt.fill_betweenx(y=[0, 1.05], x1=0.1, x2=0.4, color='gray', alpha=0.2)
# write (a) on the top right corner
plt.text(0.9, 0.93, "(a)", transform=plt.gca().transAxes, fontsize=14, verticalalignment='top', horizontalalignment='right')   

plt.xscale("log"); plt.xlim(0.01, 10); plt.ylim(0,1.05)
plt.xlabel("Temperature [GK]", fontsize=14); plt.ylabel("Fractional contribution", fontsize=14)
#plt.grid(True, ls="--", lw=0.4)
plt.legend(fontsize="small", ncol=1)
plt.tight_layout()
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.savefig(os.path.join(outdir, "TNRR_cont_both.pdf"))
plt.close()

# # ---------- 3) Contribution of each resonance with band and total with band ----------
# plt.figure(figsize=(10,7))
# colors = plt.cm.tab10(np.arange(nres))
# for i in range(nres):
#     # band for this work per resonance
#     valid_i = np.isfinite(contrib_lo_this[i]) & np.isfinite(contrib_hi_this[i]) & (contrib_hi_this[i] > 0)
#     plt.fill_between(T9, contrib_lo_this[i], contrib_hi_this[i], where=valid_i, color=colors[i], alpha=0.35)
#     plt.plot(T9, contrib_nom_this[i], color=colors[i], linewidth=0.5, label=f"E$_\\alpha= {int(Er_lab_keV_this[i])}$ keV")
# # total band & nominal (this work)
# plt.fill_between(T9, rate_lo_this, rate_hi_this, color='gray', alpha=0.35, label="") #"total ±1σ (this)"
# plt.plot(T9, rate_nom_this, color='k', linewidth=0.5, label="total (this)")
# plt.xscale("log"); plt.yscale("log"); plt.xlim(0.1, 10); plt.ylim(1e-15, 1e3)
# plt.xlabel("Temperature [GK]")
# #plt.ylabel(r"Contribution (cm$^3$ mol$^{-1}$ s$^{-1}$)")
# plt.ylabel(r"$N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
# plt.grid(True, ls="--", lw=0.4); plt.legend(fontsize="small", ncol=1)
# plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
# plt.tight_layout()
# plt.savefig(os.path.join(outdir, "3_contributions_each_with_bands_and_total.pdf"))
# plt.close()
## calculation of ratio plot
# Find indices where both T9 and T9_iliadis have matching values
# common_T9 = np.intersect1d(T9, T9_iliadis)
# # Get indices for T9 and T9_iliadis
# idx_this = np.nonzero(np.isin(T9, common_T9))[0]
# idx_iliadis = np.nonzero(np.isin(T9_iliadis, common_T9))[0]
# # Compute ratio only where both are available
# ratio = rate_nom_this[idx_this] / rate_med_iliadis[idx_iliadis]
# T9_ratio = T9[idx_this]

# # ---------- 4) Ratio: this / literature with conservative band ----------
# # Conservative ratio bounds: hi = this_hi / lit_lo , lo = this_lo / lit_hi
# ratio_nom = rate_nom_this / np.clip(rate_nom_lit, tiny, None)
# ratio_hi = (rate_hi_this) / np.clip(rate_lo_lit, tiny, None)
# ratio_lo = (rate_lo_this) / np.clip(rate_hi_lit, tiny, None)
# ratio_lo = np.maximum(ratio_lo, tiny)
# ratio_lit = rate_nom_lit / np.clip(rate_nom_lit, tiny, None)  # horizontal line at 1





# # --- Plot comparison ---
# plt.figure(figsize=(8,5))
# plt.plot(T9, rate_nom_this, color="tab:orange", lw=0.5, label="This work")
# plt.fill_between(T9, rate_lo_this, rate_hi_this, color="tab:orange", alpha=0.3)
# # Plot Iliadis 2010 median rate and its band
# plt.plot(T9_iliadis, rate_med_iliadis, color="tab:blue", lw=0.5, label="Iliadis 2010")
# plt.fill_between(T9_iliadis, rate_lo_iliadis, rate_hi_iliadis, color="tab:blue", alpha=0.3)
# # Plot NACR median rate
# plt.plot(T9_NACR, rate_med_NACR, color="tab:green", lw=0.5, label="NACR")
# #plt.fill_between(T9_NACR, rate_med_NACR*0.8, rate_med_NACR*1.2, color="tab:green", alpha=0.3)

# # Plot örres 2000 median rate
# plt.plot(T9_Gor, rate_med_Gor, color="tab:red", lw=0.5, label="Görres 2000")
# #plt.fill_between(T9_Gor, rate_med_Gor*0.8, rate_med_Gor*1.2, color="tab:red", alpha=0.3)

# plt.xscale("log"); plt.yscale("log"); plt.xlim(0.1,1.0); plt.ylim(1e-14, 1e0)
# plt.xlabel("T$_9$ (GK)"); plt.ylabel(r"$N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
# plt.legend()
# plt.grid(True, which="both", ls="--", lw=0.4)
# plt.tight_layout()
# plt.savefig("plots_all/compare_total_rate_with_iliadis_v1.pdf")
# plt.show()

# print("Saved plots to", outdir)
# plt.figure(figsize=(8,4))
# plt.plot(T9_ratio, ratio, color="k")
# plt.xscale("log"); plt.xlim(0.1,10)
# plt.xlabel("T$_9$ (GK)"); plt.ylabel("This work / Iliadis 2010")
# plt.grid(True, ls="--", lw=0.4)
# plt.tight_layout()
# plt.savefig("plots_all/ratio_this_to_iliadis.pdf")
# plt.show()


tiny = 1e-300
# --- ratio of median rates ---
ratio_med = rate_med_this / np.clip(rate_med_iliadis, tiny, None)

# --- conservative uncertainty band ---
ratio_lo = rate_low_this / np.clip(rate_hi_iliadis, tiny, None)
ratio_hi = rate_hi_this / np.clip(rate_low_iliadis, tiny, None)

# ratio_NACRE = rate_med_this / np.clip(rate_med_NACR, tiny, None)
plt.figure(figsize=(8,5))

# median ratio line
plt.plot(T9, ratio_med, color="blue",
         label=r"$\frac{\langle\sigma v\rangle_{\rm this}}{\langle\sigma v\rangle_{\rm Iliadis}}$")
plt.legend(fontsize=20)  # Increase legend font size here

# uncertainty band
plt.fill_between(T9, ratio_lo, ratio_hi, color="blue", alpha=0.10,
                 label="") #Uncertainty band
# helium burning range
#plt.fill_betweenx([0, 2.0], 0.1, 0.4, color="gray", alpha=0.2)

# plot a dotted line at y=1
plt.axhline(1.0, color="k", linestyle="--", linewidth=0.8)

plt.xscale("log")
plt.yscale("log")
plt.xlim(0.07, 2.0)
plt.ylim(0.4, 1.4)
plt.xticks([0.1, 0.2, 0.4, 0.6, 1.0, 2.0], [f"{x:.1f}" for x in [0.1, 0.2, 0.4, 0.6, 1.0, 2.0]])
# plt.yticks([0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0], [f"{y:.1f}" for y in [0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0]])
plt.yticks([0.4, 0.6, 0.8, 1.0, 1.2, 1.4], [f"{y:.1f}" for y in [0.4, 0.6, 0.8, 1.0, 1.2, 1.4]])
#plt.xticks(np.arange(0.1, 1.0, 0.1), [f"{x:.1f}" for x in np.arange(0.1, 1.1, 0.1)])
#plt.yticks(np.arange(0.4, 1.1, 0.2), [f"{y:.1f}" for y in np.arange(0.4, 1.1, 0.2)])
# plt.ylim(bottom=0.4)

plt.xlabel("Temperature [GK]")
plt.ylabel("Reaction rate ratio")

plt.legend(fontsize="large")
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.tight_layout()

plt.savefig(os.path.join(outdir, "Ratio_wband_v2.pdf"))
plt.close()
