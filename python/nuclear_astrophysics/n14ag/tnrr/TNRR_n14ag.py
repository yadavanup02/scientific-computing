#source myenv/bin/activate
# NOTE:

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os

# Resonance energy (keV)
Er_keV = 573.0        
Er1_KeV = 573
Er2_KeV = 1136
Er4_KeV = 1527
Er5_KeV = 1529
Er6_KeV = 1618
# Resonance strength (eV)
omega_gamma_ev = 4.6e-5
omega_gamma1_eV = 4.6e-5  
omega_gamma2_eV = 2.1e-4
omega_gamma3_eV = 9.0e-3
omega_gamma4_eV = 1.6
omega_gamma5_eV = 4.5e1

m1 = 4.002602  # projectile mass in amu (alpha particle)
m2 = 14.003074 # target mass in amu (example: ~14N)
mu_amu = (m1 * m2) / (m1 + m2)  # reduced mass in amu

# Convert units: keV -> MeV, eV -> MeV
Er_MeV = Er_keV * 1e-3
omega_gamma_MeV = omega_gamma_ev * 1e-6

# Temperature grid
T9 = np.round(np.arange(0.1, 5.00 + 0.01, 0.01), 2)  # GK

def narrow_resonance_rate_NA(T9_array, Er_MeV, omega_gamma_MeV, mu_amu):
    """
    Compute N_A <sigma v> for a narrow isolated resonance.
    Returns array of rates in cm^3 mol^-1 s^-1 for given T9_array.
    """
    # Avoid division by zero; T9_array assumed > 0
    prefactor = 1.5399e11    # numeric constant (when omega_gamma in MeV, Er in MeV, mu in amu, T9 in GK)
    # (1/(mu*T9))^(3/2)
    temperature_term = (1.0 / (mu_amu * T9_array)) ** 1.5
    expo = np.exp(-11.605 * Er_MeV / T9_array)
    rate = prefactor * temperature_term * omega_gamma_MeV *  expo
    return rate

rates = narrow_resonance_rate_NA(T9, Er_MeV, omega_gamma_MeV, mu_amu)

# Build a DataFrame
df = pd.DataFrame({
    "T9_GK": T9,
    "rate_cm3_mol_s": rates,
})
# add log10 column (handle zeros)
df["log10_rate"] = np.where(df["rate_cm3_mol_s"] > 0, np.log10(df["rate_cm3_mol_s"]), -np.inf)

# Display the first & last rows as quick check
print("Resonance parameters used:")
print(f"  Er = {Er_keV} keV ({Er_MeV:.6f} MeV)")
print(f"  omega*gamma = {omega_gamma_ev} eV ({omega_gamma_MeV:.6e} MeV)")
print(f"  reduced mass mu = {mu_amu:.6f} amu (m1={m1} amu, m2={m2} amu)")
print()
print("Sample of computed rate table (first 8 rows):")
print(df.head(8).to_string(index=False))
print("...")
print("Sample (last 8 rows):")
print(df.tail(8).to_string(index=False))

# ---------------------------
# Save DataFrame to txt for convenience
# ---------------------------
out_txt = "reaction_rate_table.txt"
df.to_csv(out_txt, index=False)
print(f"\nSaved rate table to: {out_txt}")

# ---------------------------
# Plot (log y-axis)
# ---------------------------
plt.figure(figsize=(8,5))
plt.plot(df["T9_GK"], df["rate_cm3_mol_s"])
plt.yscale("log")
plt.xlabel("Temperature $T_9$ (GK)")
plt.ylabel(r"$N_A\langle\sigma v\rangle$ (cm$^3$ mol$^{-1}$ s$^{-1}$)")
plt.title(f"Narrow-resonance rate\nEr={Er_keV} keV, ωγ={omega_gamma_ev} eV, μ={mu_amu:.4f} u")
plt.grid(True, which="both", ls="--", lw=0.5)
out_pdf = "reaction_rate.pdf"
plt.tight_layout()
plt.savefig(out_pdf)
plt.show()

print(f"Plot saved to: {out_pdf}")

# Also provide a small summary: temperature at which the rate is maximum
max_idx = np.nanargmax(rates)
print(f"\nMaximum rate: {rates[max_idx]:.3e} cm^3 mol^-1 s^-1 at T9 = {T9[max_idx]:.2f} GK")
