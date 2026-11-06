#!/usr/bin/env python3
"""
Compute narrow-resonance thermonuclear reaction rates for multiple resonances
(alpha + 14N example). Produces:
 - tnrr_table.csv      : T9, per-resonance rates, total, and log10 columns (NaN where rate==0)
 - tnrr_fractional.pdf : fractional contributions vs T9 (0.1-10 GK)
 - tnrr_total.pdf      : total TNRR vs T9 (log-log)
 - tnrr_contribs_total.pdf : individual contributions + total on same log-log plot

Author: ChatGPT (snippet adapted for user's resonances)
"""

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os
import matplotlib
matplotlib.use('Agg') 

# ---------------------------
# Resonance list (user-provided)
# Energies are alpha LAB energies in keV, wg in eV
# ---------------------------
Er_keV = np.array([573.0, 1136.0, 1395.0, 1532.0, 1618.0])
wg_eV   = np.array([4.5e-5, 2.1e-2, 9.0e-3, 1.6, 0.45])

# ---------------------------
# Unit conversions
# ---------------------------
Er_MeV_lab = Er_keV * 1e-3      # keV -> MeV
wg_MeV     = wg_eV * 1e-6       # eV -> MeV

# ---------------------------
# Temperature grid
# ---------------------------
T9 = np.round(np.arange(0.01, 10.00 + 0.01, 0.01), 2)  # GK

# ---------------------------
# Physical constants / masses
# ---------------------------
# Prefactor used in many references (matching your gnuplot)
prefactor = 1.5394e11  # when wg in MeV, Er in MeV, mu in amu, T9 in GK -> NA<σv> in cm^3 mol^-1 s^-1

# Use realistic atomic masses (alpha and 14N)
m_alpha = 4.002602      # u
m_14N   = 14.003074     # u

def reduced_mass(m1, m2):
    return (m1 * m2) / (m1 + m2)

mu_alpha_14 = reduced_mass(m_alpha, m_14N)

# ---------------------------
# Convert lab alpha energy -> center-of-mass energy like your gnuplot:
# Er_cm = Er_lab * m_target / (m_target + m_projectile)
# (here project = alpha, target = 14N)
# ---------------------------
conv_factor = m_14N / (m_14N + m_alpha)
Er_MeV_cm = Er_MeV_lab * conv_factor

# ---------------------------
# Narrow-resonance vectorised function
# returns shape (n_res, n_T)
# ---------------------------
def narrow_resonance_rates(T9_array, Er_cm_MeV_arr, wg_MeV_arr, mu_amu, prefactor=prefactor):
    T = T9_array[np.newaxis, :]            # shape (1, nT)
    wg = wg_MeV_arr[:, np.newaxis]         # shape (nres, 1)
    Er = Er_cm_MeV_arr[:, np.newaxis]      # shape (nres, 1)
    temperature_term = (1.0 / (mu_amu * T)) ** 1.5
    expo = np.exp(-11.605 * Er / T)        # 11.605 = 1/(k_B in MeV/GK)
    rate = prefactor * wg * temperature_term * expo
    return rate  # shape (nres, nT)

# ---------------------------
# Compute rates (preferred interpretation)
# ---------------------------
rates = narrow_resonance_rates(T9, Er_MeV_cm, wg_MeV, mu_alpha_14)  # shape (n_res, nT)
total_rate = np.sum(rates, axis=0)                                   # shape (nT,)

# ---------------------------
# Build DataFrame and handle logs (avoid log10(0) warnings)
# ---------------------------
cols = ["T9_GK"] + [f"r{i+1}" for i in range(len(Er_keV))] + ["total"]
data = np.vstack([T9, rates, total_rate]).T
df = pd.DataFrame(data, columns=cols)

# for readability add original inputs as metadata columns (not strictly necessary)
df["T9_GK"] = df["T9_GK"].astype(float)
# for c in cols[1:]:
#     # create log10 columns but use NaN where rate == 0 to avoid -inf/warnings
#     df[f"log10_{c}"] = np.where(df[c] > 0, np.log10(df[c]), np.nan)
with np.errstate(divide='ignore', invalid='ignore'):
    # compute log columns in a safe way (repeat for each col if needed)
    for c in cols[1:]:
        df[f"log10_{c}"] = np.where(df[c] > 0, np.log10(df[c]), np.nan)


# Save CSV
out_csv = "tnrr_table.csv"
df.to_csv(out_csv, index=False)
print("Saved table to:", out_csv)

# ---------------------------
# Plot 1: fractional contributions (0.1 - 10 GK, linear y)
# ---------------------------
os.makedirs("plots", exist_ok=True)
try:
    plt.figure(figsize=(8,5))
    for i in range(rates.shape[0]):
        frac = rates[i,:] / total_rate
        plt.plot(T9, frac, label = f"$E_\\alpha={Er_keV[i]:.0f}\\ \\mathrm{{keV}},\\ \\omega\\gamma={wg_eV[i]:g}\\ \\mathrm{{eV}}$")
    # valid mathtext label: use a normal f-string, escape backslashes

    plt.xscale("log")
    plt.xlim(0.1, 10)
    plt.ylim(0, 1.05)
    plt.xlabel("T$_9$ (GK)")
    plt.ylabel("Fractional contribution")
    #plt.grid(True, which="both", ls="--", lw=0.4)
    #plt.legend(fontsize="small", loc="best")
    plt.legend(fontsize="small", loc="center right")
    # how to set ticks inside plot area:
    plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
    plt.tight_layout()
    plt.savefig("plots/tnrr_fractional_ng.pdf")
    plt.show()
    print("Saved plot: plots/tnrr_fractional_ng.pdf")
    plt.close()   # important
except Exception as e:
    print("Warning: fractional plot failed:", e)
# ---------------------------
# Plot 2: total TNRR (log-log)
# ---------------------------
try:
    plt.figure(figsize=(8,5))
    plt.plot(T9, total_rate, label="Total TNRR (sum of resonances)")
    plt.xscale("log")
    plt.yscale("log")
    plt.xlim(0.01, 10)
    plt.ylim(1e-14, 1e3)
    plt.xlabel("T$_9$ (GK)")
    plt.ylabel(r"Total $N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
    #plt.grid(True, which="both", ls="--", lw=0.4)
    plt.legend()
    plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
    plt.tight_layout()
    plt.savefig("plots/tnrr_total_ng.pdf")
    plt.show()
    print("Saved plot: plots/tnrr_total_ng.pdf")
    plt.close()   # important
except Exception as e:
    print("Warning: total rate plot failed:", e)

# ---------------------------
# Plot 3: contributions + total on same log-log plot
# ---------------------------
plt.figure(figsize=(9,6))
for i in range(rates.shape[0]):
    plt.plot(T9, rates[i,:], label=f"r{i+1} ({Er_keV[i]:.0f} keV)")
plt.plot(T9, total_rate, color="k", linewidth=1.8, label="total")
plt.xscale("log")
# plt.yscale("log")
plt.xlim(0.1, 10)
#plt.ylim(1e-14, )
plt.xlabel("T$_9$ (GK)")
plt.ylabel(r"$N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
#plt.grid(True, which="both", ls="--", lw=0.4)
plt.legend(fontsize="small")
plt.tight_layout()
plt.gca().tick_params(axis='both', which='both', direction='in', top=True, right=True)
plt.savefig("plots/tnrr_contribs_total_ng.pdf")
plt.show()
print("Saved plot: plots/tnrr_contribs_total_ng.pdf")

# ---------------------------
# Quick textual summary
# ---------------------------
print("\nPeaks (per resonance):")
for i in range(rates.shape[0]):
    idx = np.nanargmax(rates[i,:])
    print(f" r{i+1}: peak = {rates[i,idx]:.3e} cm^3/mol/s at T9 = {T9[idx]:.2f} GK (lab Er = {Er_keV[i]:.1f} keV)")
idx_tot = np.nanargmax(total_rate)
print(f" total: peak = {total_rate[idx_tot]:.3e} cm^3/mol/s at T9 = {T9[idx_tot]:.2f} GK")

# show first rows
print("\nFirst rows of the saved table:")
print(df.head(8).to_string(index=False))
