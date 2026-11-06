import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os

# User-provided resonances (from your message): energies in keV, wg in eV
Er_keV = np.array([573.0, 1136.0, 1395.0, 1532.0, 1618.0])
wg_eV   = np.array([4.5e-5, 2.1e-4, 9.0e-3, 1.6, 4.5e1])
# Er_keV = np.array([573.0, 1136.0])
# wg_eV   = np.array([4.5e-5, 2.1e-4])

# Convert units
Er_MeV = Er_keV * 1e-3
wg_MeV = wg_eV * 1e-6  # eV -> MeV

# Temperature grid
T9 = np.round(np.arange(0.01, 10.00 + 0.01, 0.01), 2)

# Narrow-resonance prefactor
prefactor = 1.5394e11  # match your gnuplot value

# Two mass choices
m_proj_wrong = 4.002602   # what I (incorrectly) used earlier (proton)
m_targ_wrong = 14.003074

m_proj_alpha = 4.002602       # to match your gnuplot (an=14, ahe=4)
m_targ_alpha = 14.003074

def reduced_mass(m1, m2):
    return (m1 * m2) / (m1 + m2)

mu_wrong = reduced_mass(m_proj_wrong, m_targ_wrong)
mu_alpha = reduced_mass(m_proj_alpha, m_targ_alpha)

def narrow_resonance_rate(T9_arr, Er_cm_MeV, wg_MeV_arr, mu_amu, prefactor=prefactor):
    """Vectorised narrow-resonance rate for multiple resonances (wg array)"""
    # shape handling: if wg is array, returns shape (n_res, n_T)
    T = T9_arr[np.newaxis, :]  # shape (1, nT)
    wg = wg_MeV_arr[:, np.newaxis]  # shape (nres,1)
    Er = Er_cm_MeV[:, np.newaxis]   # shape (nres,1)
    temp_term = (1.0 / (mu_amu * T)) ** 1.5
    expo = np.exp(-11.605 * Er / T)
    rate = prefactor * wg * temp_term * expo
    return rate  # shape (nres, nT)

# Compute two interpretations for energies:
# (A) energies are center-of-mass (Er_cm = Er_MeV)
Er_cm_A = Er_MeV.copy()

# (B) energies are projectile (lab) energies: convert to CM using m_target/(m_target+m_proj)
# Use alpha+14 values to compute the conversion factor like your gnuplot's er(ea)
conv_factor = m_targ_alpha / (m_targ_alpha + m_proj_alpha)
Er_cm_B = Er_MeV * conv_factor

# Rates: wrong mass (what I used earlier) vs correct alpha mass
rates_wrong_A = narrow_resonance_rate(T9, Er_cm_A, wg_MeV, mu_wrong)  # wrong mass, Er as CM
rates_alpha_A = narrow_resonance_rate(T9, Er_cm_A, wg_MeV, mu_alpha)  # alpha mass, Er as CM

rates_wrong_B = narrow_resonance_rate(T9, Er_cm_B, wg_MeV, mu_wrong)  # wrong mass, Er as LAB->CM
rates_alpha_B = narrow_resonance_rate(T9, Er_cm_B, wg_MeV, mu_alpha)  # alpha mass, Er as LAB->CM

# totals
total_wrong_A = np.sum(rates_wrong_A, axis=0)
total_alpha_A = np.sum(rates_alpha_A, axis=0)
total_wrong_B = np.sum(rates_wrong_B, axis=0)
total_alpha_B = np.sum(rates_alpha_B, axis=0)

# Build DataFrame for the preferred case (alpha mass, lab->CM conversion, which matches your gnuplot)
# Choose B (convert lab to CM) because your gnuplot does er(ea) = ea * an/(an+ahe) before the exponential.
rates_pref = rates_alpha_B  # shape (nres, nT)
total_pref = total_alpha_B

cols = ["T9_GK"] + [f"r{i+1}" for i in range(len(Er_keV))] + ["total"]
data = np.vstack([T9, rates_pref, total_pref])
df = pd.DataFrame(data.T, columns=cols)
for c in cols[1:]:
    df[f"log10_{c}"] = np.where(df[c] > 0, np.log10(df[c]), -np.inf)

out_csv = "tnrr_checked_table.csv"
df.to_csv(out_csv, index=False)

# ---- PLOTS ----
# 1) Fractional contributions (preferred case: alpha mass, lab->CM conversion)
plt.figure(figsize=(8,5))
for i in range(rates_pref.shape[0]):
    frac = rates_pref[i,:] / total_pref
    plt.plot(T9, frac, label=fr"$E_\alpha={Er_keV[i]:.0f}$ keV, $\omega\gamma={wg_eV[i]:g}$ eV")
plt.xlabel("T9 (GK)")
plt.ylabel("Fractional contribution")
plt.xscale("log")
plt.xlim(0.1, 10)
plt.yscale("linear")
#plt.title("Fractional contributions (alpha mass, lab->CM conversion)")
plt.grid(True, which="both", ls="--", lw=0.5)
plt.legend(fontsize="small", loc="best")
out_frac = "tnrr_fractional.pdf"
plt.tight_layout()
plt.savefig(out_frac)
plt.show()

# Print summary
print("Saved CSV:", out_csv)
print("Saved plots:")
print(" - Fractions:  ", out_frac)
for i in range(len(Er_MeV)):
    idx = np.nanargmax(rates_pref[i])
    print(f" r{i+1}: peak = {rates_pref[i,idx]:.3e} cm^3/mol/s at T_9 = {T9[idx]:.2f} GK (Er_lab={Er_MeV[i]:.3f} MeV)")
idx_tot = np.nanargmax(total_pref)
print(f" total: peak = {total_pref[idx_tot]:.3e} cm^3/mol/s at T_9 = {T9[idx_tot]:.2f} GK")

# Also print the typical ratio between my earlier wrong-mu and correct alpha-mu at a few T9 values
for tval in [0.1, 0.5, 1.0, 2.0, 5.0]:
    i = int(np.round((tval - 0.01) / 0.01))
    print(f" T9={tval:.2f}: wrong/alpha = {total_wrong_B[i]/total_alpha_B[i]:.3f}")

# show first rows of table
print("\nFirst 8 rows of result table:")
print(df.head(8).to_string(index=False))

