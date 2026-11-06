#!/usr/bin/env python3
"""
plot_compare.py

General-purpose converter of RatesMC plotting: reads RatesMC output files
(e.g. RatesMC.cont, RatesMC.out, RatesMC.full or whitespace/CSV data) and
produces a comparison plot with median and uncertainty band.

Usage:
  python plot_compare.py           # auto-find files in cwd
  python plot_compare.py -i FILE   # specify an input file
  python plot_compare.py -o out.pdf
"""

import argparse
import sys
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# -------------------------
# Helpers to read files
# -------------------------
def read_numeric_table(path: Path):
    """Try to read a whitespace-separated numeric table, ignoring comment lines that start with '#'.
       Returns a pandas DataFrame or raises.
    """
    # Try pandas read_table with comment char
    try:
        df = pd.read_csv(path, comment='#', delim_whitespace=True, header=None)
        if df.shape[1] < 2:
            raise ValueError("Not enough columns")
        return df
    except Exception as e:
        # Try CSV-style
        try:
            df = pd.read_csv(path, comment='#')
            if df.shape[1] < 2:
                raise
            return df
        except Exception as e2:
            raise RuntimeError(f"Failed to parse {path}: {e}; {e2}")

def auto_identify_columns(df: pd.DataFrame):
    """
    Attempt to identify temperature and rate columns.
    Strategy:
      - If df has >=4 columns, assume:
          col0 = T, col1 = low, col2 = median, col3 = high
      - If 3 columns: col0=T, col1=median, col2=uncert (or high). Try to treat col1 median, col2 high.
      - If 2 columns: col0=T, col1=median (no uncertainty)
      - If header present and contains 'T' or 'Temp' or 'T9' use that.
    Returns dict with keys: 'T', 'median', optional 'low','high'
    """
    colnames = list(df.columns)
    # If columns have names (strings), try to match common words
    if df.columns.dtype == object:
        lower_names = [str(c).lower() for c in colnames]
        # try T
        for i, name in enumerate(lower_names):
            if any(k in name for k in ("t", "temp", "t9", "temperature")):
                Tcol = i
                break
        else:
            Tcol = 0
        # try median/median_rate
        median_col = None
        for i, name in enumerate(lower_names):
            if any(k in name for k in ("median","rate","median_rate","central")):
                median_col = i
                break
        if median_col is None:
            # fallback: second column
            median_col = 1 if len(colnames) > 1 else 0
    else:
        Tcol = 0
        median_col = 1 if df.shape[1] > 1 else 0

    result = {'T': df.iloc[:, Tcol].to_numpy()}

    ncol = df.shape[1]
    # heuristic for low/median/high ordering
    if ncol >= 4:
        result['low'] = df.iloc[:, 1].to_numpy()
        result['median'] = df.iloc[:, 2].to_numpy()
        result['high'] = df.iloc[:, 3].to_numpy()
    elif ncol == 3:
        # T, median, high (or T, low, high) -> assume median = col1, high = col2
        result['median'] = df.iloc[:, 1].to_numpy()
        result['high'] = df.iloc[:, 2].to_numpy()
    elif ncol == 2:
        result['median'] = df.iloc[:, 1].to_numpy()
    else:
        raise ValueError("Unrecognized column layout")

    return result

# -------------------------
# Plotting
# -------------------------
def plot_rate(data, outpath="Compare.pdf", title=None, loglog=True, show=False):
    T = np.array(data['T'])
    med = np.array(data['median'])
    low = np.array(data.get('low')) if 'low' in data else None
    high = np.array(data.get('high')) if 'high' in data else None

    # prepare figure
    fig, ax = plt.subplots(figsize=(7.2,5))
    if loglog:
        ax.set_xscale('log')
        ax.set_yscale('log')
    ax.plot(T, med, label='median', linewidth=1.5)
    if low is not None and high is not None:
        ax.fill_between(T, low, high, alpha=0.3, label='low--high band')
    elif low is not None:
        ax.fill_between(T, med - low, med + low, alpha=0.3, label='±uncert')
    ax.set_xlabel('Temperature (GK)')
    ax.set_ylabel('Reaction rate (units from file)')
    if title:
        ax.set_title(title)
    ax.grid(True, which='both', ls='--', lw=0.3)
    ax.legend()
    plt.tight_layout()
    fig.savefig(outpath)
    print(f"Saved plot to {outpath}")
    if show:
        plt.show()
    plt.close(fig)

# -------------------------
# Main
# -------------------------
def main():
    p = argparse.ArgumentParser(description="Plot RatesMC-like outputs (median + uncertainty band).")
    p.add_argument('-i','--input', help='Input data file (RatesMC.cont / RatesMC.out / RatesMC.full). If omitted we try to auto-find.', default=None)
    p.add_argument('-o','--output', help='Output PDF file', default='Compare.pdf')
    p.add_argument('--show', action='store_true', help='Show interactive plot window')
    p.add_argument('--loglog', action='store_true', help='Use log-log scales (recommended for rates)')
    args = p.parse_args()

    cwd = Path('.')
    candidates = []
    if args.input:
        candidates = [Path(args.input)]
    else:
        for name in ['RatesMC.cont','RatesMC.out','RatesMC.full','RatesMC.samp','RatesMC.fulltxt']:
            pth = cwd / name
            if pth.exists():
                candidates.append(pth)
        # also pick any *.cont or *.out file
        candidates += list(cwd.glob('*.cont')) + list(cwd.glob('*.out')) + list(cwd.glob('*.dat'))
    candidates = [c for c in candidates if c.exists()]
    if not candidates:
        print("No candidate input files found in current directory. Provide -i <file>.", file=sys.stderr)
        sys.exit(2)

    # pick the first readable candidate
    data = None
    last_error = None
    for cand in candidates:
        try:
            df = read_numeric_table(cand)
            parsed = auto_identify_columns(df)
            parsed['source'] = str(cand)
            data = parsed
            break
        except Exception as e:
            last_error = e
    if data is None:
        raise RuntimeError(f"Failed to read any candidate file. Last error: {last_error}")

    title = f"RatesMC: {Path(data['source']).name}"
    plot_rate(data, outpath=args.output, title=title, loglog=args.loglog, show=args.show)

if __name__ == '__main__':
    main()
