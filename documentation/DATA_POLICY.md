# Data publication policy

This repository is public source code, not an experimental-data archive.

## Do not commit

- raw detector-acquisition files, binary files, and ROOT data files;
- run-specific beam, current, pressure, or detector logs;
- unpublished count tables, yields, fit results, or resonance-strength values;
- laboratory-internal paths, credentials, or configuration files;
- generated figures that reveal unpublished results.

The `.gitignore` intentionally excludes common forms of these files.

## Safe additions

Only add data after confirming that it may be shared publicly. Prefer one of these approaches:

- a synthetic example with invented values;
- a small, explicitly approved public dataset with its source cited;
- a documented input schema with no values.

## Before publishing a macro

1. Check it for absolute paths and replace them with configurable paths.
2. Review comments, plot labels, filenames, and output directories for unpublished information.
3. Confirm that any embedded numbers may be public.
4. State the expected input format and generated outputs in the macro documentation.
