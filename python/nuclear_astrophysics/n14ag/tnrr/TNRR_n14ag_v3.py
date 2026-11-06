
# # ---------------------------
# # User inputs: resonances & uncertainties
# # Energies in keV, strengths in eV
# # ---------------------------
# Er_keV = np.array([573.0, 1136.0, 1395.0, 1532.0, 1618.0])
# wg_eV   = np.array([4.5e-5, 2.1e-2, 9.0e-3, 1.6, 0.45])

# # 1-sigma absolute uncertainties (set to 0 if unknown)
# dEr_keV = np.array([3.0, 3.0, 3.0, 2.0, 2.0])    # example uncertainties in keV
# dwg_eV  = np.array([0.3e-5, 0.3e-3, 1.0e-3, 0.13, 0.05])  # example uncertainties in eV

#!/usr/bin/env python3
"""
Simplified tnrr_with_uncertainty: nominal curves + 16-84% uncertainty bands
Uses log-space percentiles for robust bands.
"""

import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os

# ---------- User inputs ----------
Er_keV = np.array([573.0, 1136.0, 1395.0, 1532.0, 1618.0])
wg_eV   = np.array([4.5e-5, 2.1e-2, 9.0e-3, 1.6, 0.45])   # you had these values

dEr_keV = np.array([3.0, 3.0, 3.0, 2.0, 2.0])
dwg_eV  = np.array([0.3e-5, 0.3e-3, 1.0e-3, 0.13, 0.05])

N_mc = 4000

# ---------- Units & grid ----------
Er_MeV_lab = Er_keV * 1e-3
wg_eV = np.array(wg_eV)
wg_MeV = wg_eV * 1e-6
dEr_MeV = dEr_keV * 1e-3
dwg_MeV = dwg_eV * 1e-6

T9 = np.round(np.arange(0.01, 10.00 + 0.01, 0.01), 2)
nT = len(T9)
nres = len(Er_keV)

# ---------- Physics constants ----------
prefactor = 1.5394e11
m_alpha = 4.002602
m_14N   = 14.003074
mu = (m_alpha * m_14N) / (m_alpha + m_14N)
conv = m_14N / (m_14N + m_alpha)   # lab alpha -> CM
Er_MeV_cm = Er_MeV_lab * conv
dEr_MeV_cm = dEr_MeV * conv

# ---------- rate function ----------
def narrow_rates(Er_cm_MeV_arr, wg_MeV_arr, T9_arr=T9, mu_amu=mu, pref=prefactor):
    T = T9_arr[np.newaxis, :]                # (1, nT)
    Er = Er_cm_MeV_arr[:, np.newaxis]        # (nres,1)
    wg = wg_MeV_arr[:, np.newaxis]           # (nres,1)
    temp_term = (1.0 / (mu_amu * T)) ** 1.5
    expo = np.exp(-11.605 * Er / T)
    return pref * wg * temp_term * expo      # (nres, nT)

# ---------- nominal rates (once) ----------
rates_nominal = narrow_rates(Er_MeV_cm, wg_MeV)   # (nres, nT)
total_nominal = rates_nominal.sum(axis=0)

# ---------- Monte-Carlo sampling ----------
rng = np.random.default_rng(12345)
total_samples = np.zeros((N_mc, nT))
per_res_samples = np.zeros((nres, N_mc, nT))   # keep to build per-res percentiles

for k in range(N_mc):
    # Er sample (normal)
    Er_samp_cm = rng.normal(Er_MeV_cm, dEr_MeV_cm)
    # wg sample: lognormal when dwg>0, else normal/truncate
    wg_samp_eV = np.zeros(nres)
    for i in range(nres):
        if (wg_eV[i] > 0) and (dwg_eV[i] > 0):
            rel = dwg_eV[i] / wg_eV[i]
            sigma_ln = np.sqrt(np.log(1.0 + rel**2))
            mu_ln = np.log(wg_eV[i])
            samp_eV = rng.lognormal(mean=mu_ln, sigma=sigma_ln)
        else:
            # fallback to non-negative normal
            samp_eV = rng.normal(wg_eV[i], dwg_eV[i]) if dwg_eV[i] > 0 else wg_eV[i]
            if samp_eV < 0:
                samp_eV = 0.0
        wg_samp_eV[i] = samp_eV
    wg_samp_MeV = wg_samp_eV * 1e-6

    rates_k = narrow_rates(Er_samp_cm, wg_samp_MeV)   # (nres, nT)
    per_res_samples[:, k, :] = rates_k
    total_samples[k, :] = rates_k.sum(axis=0)

# ---------- percentiles in log-space for stable bands ----------
tiny = 1e-300
total_samples_clipped = np.clip(total_samples, a_min=tiny, a_max=None)
log_total = np.log10(total_samples_clipped)         # (N_mc, nT)
log_p16 = np.percentile(log_total, 16, axis=0)
log_med = np.percentile(log_total, 50, axis=0)
log_p84 = np.percentile(log_total, 84, axis=0)
total_p16 = 10.0 ** log_p16
total_med = 10.0 ** log_med
total_p84 = 10.0 ** log_p84

# per-res medians and bands (log-space)
per_res_med = np.zeros((nres, nT))
per_res_p16 = np.zeros((nres, nT))
per_res_p84 = np.zeros((nres, nT))
for i in range(nres):
    arr = np.clip(per_res_samples[i], a_min=tiny, a_max=None)   # (N_mc, nT)
    logarr = np.log10(arr)
    per_res_p16[i] = 10.0 ** np.percentile(logarr, 16, axis=0)
    per_res_med[i] = 10.0 ** np.percentile(logarr, 50, axis=0)
    per_res_p84[i] = 10.0 ** np.percentile(logarr, 84, axis=0)

# ---------- save summary table ----------
os.makedirs("plots", exist_ok=True)
pd.DataFrame({
    "T9_GK": T9,
    "total_med": total_med,
    "total_p16": total_p16,
    "total_p84": total_p84
}).to_csv("tnrr_table_median_percentiles.csv", index=False)
print("Saved table: tnrr_table_median_percentiles.csv")

# ---------- Plot: total nominal + median + band ----------
valid = np.isfinite(total_p16) & np.isfinite(total_med) & np.isfinite(total_p84) & (total_p84 > 0)
p16_plot = np.where(total_p16 > 0, total_p16, tiny)
p84_plot = np.where(total_p84 > 0, total_p84, tiny)
med_plot = np.where(total_med > 0, total_med, tiny)
nom_plot = np.where(total_nominal > 0, total_nominal, tiny)

plt.figure(figsize=(8,5))
#plt.plot(T9, nom_plot, label="Total (nominal)", color="k", linewidth=0.5)
plt.plot(T9, med_plot, linestyle="--", label="Total (MC median)", color="gray")
plt.fill_between(T9, p16_plot, p84_plot, where=valid, alpha=0.50, interpolate=True, label="16–84% (MC band)")
#plt.xscale("log")
plt.yscale("log")
plt.xlim(0.1, 1)
plt.ylim(1e-14, 1e3)
plt.xlabel("T9 (GK)")
plt.ylabel(r"Total $N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
plt.grid(True, which="both", ls="--", lw=0.4); plt.legend()
plt.tight_layout(); plt.savefig("plots/tnrr_total_nominal_and_band.pdf"); plt.close()
print("Saved: plots/tnrr_total_nominal_and_band.pdf")

# ---------- Plot: per-res contributions with bands + nominal total ----------
plt.figure(figsize=(9,6))
colors = plt.cm.tab10(np.arange(nres))
for i in range(nres):
    valid_i = np.isfinite(per_res_p16[i]) & np.isfinite(per_res_med[i]) & np.isfinite(per_res_p84[i]) & (per_res_p84[i] > 0)
    p16_i = np.where(per_res_p16[i] > 0, per_res_p16[i], tiny)
    p84_i = np.where(per_res_p84[i] > 0, per_res_p84[i], tiny)
    med_i = np.where(per_res_med[i] > 0, per_res_med[i], tiny)
    nom_i = np.where(rates_nominal[i] > 0, rates_nominal[i], tiny)
    plt.fill_between(T9, p16_i, p84_i, where=valid_i, color=colors[i], alpha=0.18, interpolate=True)
    plt.plot(T9, nom_i, color=colors[i], linewidth=1.2, label=f"r{i+1} nominal {Er_keV[i]:.0f} keV")
    plt.plot(T9, med_i, linestyle="--", color=colors[i], linewidth=0.9)
plt.plot(T9, nom_plot, color="k", linewidth=2.0, label="total nominal")
plt.xscale("log"); plt.yscale("log"); plt.xlim(0.01, 10)
plt.xlabel("T9 (GK)"); plt.ylabel(r"$N_A\langle\sigma v\rangle$")
plt.grid(True, which="both", ls="--", lw=0.4); plt.legend(fontsize="small", loc="upper left")
plt.tight_layout(); plt.savefig("plots/tnrr_contribs_total_nominal_and_bands.pdf"); plt.close()
print("Saved: plots/tnrr_contribs_total_nominal_and_bands.pdf")
