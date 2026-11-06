# Scientific Computing for Nuclear Astrophysics

ROOT and Gnuplot analysis scripts from the Felsenkeller `^{14}N(\alpha,\gamma)^{18}F` workflow. The repository is intended to make the analysis code easier to inspect, reuse, and improve while keeping experimental data private.

## Contents

```text
root/nuclear_astrophysics/n14ag/
  addback/                Addback-spectrum and anti-Compton analysis
  calibration/            Energy-calibration macros
  detector_efficiency/    Detector-efficiency calculations and plots
  peak_counting/          Peak-count extraction and count-versus-run tools
  plotting/               Spectra, multi-panel, and scan plotting macros
  resonance_scans/        Resonance-scan, yield-curve, and fit macros
  resonance_strength/     Resonance-strength calculations
  utilities/              Data-reading and supporting utilities
  legacy/                 Historical scripts retained for traceability
tools/gnuplot/calorimeter/
                           Calorimeter-current, pressure, flow, and averaging scripts
python/nuclear_astrophysics/n14ag/tnrr/
                           Thermonuclear reaction-rate analysis and plotting scripts
third_party/ratesmc/        RatesMC source and its upstream GPL-3.0 license
documentation/            Project overview and data-publication policy
```

## Requirements

- [ROOT](https://root.cern/) with C++ macro support
- Gnuplot for scripts in `tools/gnuplot/calorimeter/`
- Python 3 with NumPy, pandas, Matplotlib, and SciPy for `python/.../tnrr/`
- CMake, a C++14 compiler, and GSL for `third_party/ratesmc/`

The imported scripts use ROOT classes such as `TCanvas`, `TGraphErrors`, and `TF1`.

## Data and paths

No experimental data, ROOT output files, generated plots, or analysis-result text files are included. Many original macros still contain laboratory-specific absolute paths such as `/felsdata/...`; this first import preserves their scientific logic unchanged.

Use a private local directory for data and outputs. A later refactoring pass will replace absolute paths with function arguments or a small configuration file, beginning with the resonance-scan macros.

## Running a macro

Start ROOT from the repository root and run a macro with the arguments required by its function. For example, a scan macro generally follows this pattern:

```bash
root -l 'root/nuclear_astrophysics/n14ag/resonance_scans/scan_573keV.C(start_run, end_run)'
```

Replace `start_run` and `end_run` with valid run numbers, and adapt the input/output paths before execution. Consult the macro source and [project overview](documentation/OVERVIEW.md) for the current workflow grouping.

## Initial-import policy

- Every unique source macro from the selected local folder is retained.
- Exact duplicate copies are represented once; the originals on the laptop are not changed.
- Distinct versioned macros are retained because they may encode meaningful analysis choices.
- Legacy files are isolated rather than deleted.
- The `plot_n14pg_*` names are historical filenames. Their code refers to the `^{14}N(\alpha,\gamma)^{18}F` scan workflow and is organized under `n14ag`.

See [data policy](documentation/DATA_POLICY.md) before adding any datasets or results.
For the thermonuclear reaction-rate code, see [TNRR notes](documentation/TNRR.md).
