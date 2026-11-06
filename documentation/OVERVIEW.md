# Project overview

## Scope

This initial import contains source code from a Felsenkeller `^{14}N(\alpha,\gamma)^{18}F` analysis workspace. The code is grouped by its role in the workflow, not by the original desktop-folder layout.

## Analysis workflow

```text
Detector acquisition / ROOT files (private)
             |
             v
Calibration -> spectra and peak counting -> addback / efficiency corrections
             |                                 |
             +------------> resonance scans ---+
                                      |
                                      v
                            yield curves and resonance strength
```

## Directory guide

| Directory | Purpose | Typical inputs | Typical outputs |
| --- | --- | --- | --- |
| `calibration/` | Energy calibration | Calibration spectra and reference peaks | Calibration coefficients and plots |
| `detector_efficiency/` | Detector-efficiency analysis | Reference source or simulation results | Efficiency curves and fit parameters |
| `peak_counting/` | Peak integration and count extraction | ROOT spectra or run data | Peak areas and count summaries |
| `addback/` | Addback and anti-Compton spectrum processing | Detector-level spectra | Addback spectra and peak counts |
| `resonance_scans/` | Scan fitting and yield analysis | Beam-energy tables and peak counts | Resonance curves, fit values, and plots |
| `resonance_strength/` | Strength calculation | Yields, efficiencies, stopping data | Resonance-strength results |
| `plotting/` | Figures for scans, spectra, and theses | Results from the preceding steps | PDF/PNG figures |
| `utilities/` | Supporting input/readout helpers | Experiment-specific files | In-memory data or ROOT objects |
| `legacy/` | Older implementations | Varies | Varies; retain for comparison only |

## Important limitations

1. This is a source-only import. The private data required by most macros is intentionally not present.
2. The first import does not alter physics calculations, fit models, cuts, or numeric constants.
3. Version suffixes such as `_v1`, `_v2`, `_paper`, and `_thesis` are retained until their scientific differences have been reviewed.
4. Some files have historical or misleading names. In particular, the `plot_n14pg_*` scripts use the alpha-capture scan workflow and are not used as evidence of a proton-capture classification.

## Next refactoring target

The first refactoring pass should cover the resonance-scan macros for 278, 430, 573, 1058, 1136, and 1618 keV. The work will:

- replace absolute paths with explicit input/output parameters;
- validate file opening and input columns;
- use consistent function and output names;
- retain existing fit functions and initial parameters unless a documented scientific reason requires a change;
- add a per-macro input/output reference.
