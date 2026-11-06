# Thermonuclear reaction-rate analysis

## Included code

`python/nuclear_astrophysics/n14ag/tnrr/` contains the preserved Python versions used to calculate, compare, and plot thermonuclear reaction-rate results for the `^{14}N(\alpha,\gamma)^{18}F` workflow. They use NumPy, pandas, Matplotlib, and, where needed, SciPy.

The scripts refer to local `RatesMC.out`, `RatesMC.cont`, and literature-comparison tables. Those files are deliberately not included because they are generated outputs or may contain unpublished analysis results.

`third_party/ratesmc/` contains the source, CMake build configuration, analysis scripts, documentation, and the generic `RatesMC_template.in` example for the RatesMC Monte Carlo program. Its original `README.md` identifies the upstream project and prerequisites.

## Running RatesMC

The bundled source requires CMake, a C++14 compiler, and GSL. Before configuring it, make a private `RatesMC.in` from the generic template and obtain the public AME/NuBase reference mass tables required by the original RatesMC build (`mass_1.mas20` and `nubase_3.mas20`). Those run-specific and reference data files are not mirrored here.

Build outside the source directory:

```bash
cd third_party/ratesmc
cp RatesMC_template.in RatesMC.in
cmake -S . -B build
cmake --build build
```

Add only data approved for public sharing to the private input file. The generated outputs are ignored by Git.

## License notice

RatesMC is third-party code distributed under GPL-3.0; its original `LICENSE` is retained in `third_party/ratesmc/`. The rest of this repository has no implied license. Keep RatesMC separate and preserve its license and attribution in any redistribution or modification.

## Publication policy

The following are intentionally excluded:

- custom reaction inputs and run-specific `.in` files;
- Monte Carlo samples, logs, rate tables, and contribution files;
- mass tables bundled with local runs;
- generated PDF/PNG figures and comparison tables.

Use synthetic or explicitly approved public inputs for an executable example.
