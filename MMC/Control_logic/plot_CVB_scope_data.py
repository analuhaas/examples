"""
Plot the ScopeMimicry data recorded by the CVB control logic example.

When you press `r` in the serial monitor, the OwnTech monitor filter saves the
scope data in `src/Data_records/` as `<date>_<time>-record.csv` (and `.txt`).
This script reads one of these files and plots, for each arm, the number of
modules to connect (N_u or N_l) and the gate signals of its 3 modules.

Usage (from the root of your OwnTech project, where the `src` folder is):

    python plot_CVB_scope_data.py                  # plots the latest record
    python plot_CVB_scope_data.py path/to/record.csv
    python plot_CVB_scope_data.py --dt 0.01 --no-show

Requirements: numpy, pandas, matplotlib (pip install numpy pandas matplotlib).

The figures are saved next to the record file, as
`<record>_upper_arm.png` and `<record>_lower_arm.png`.
"""

import argparse
import glob
import os

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Time between two scope points: scope_period (100) * control_task_period (100 us) = 10 ms
DEFAULT_DT = 0.01  # [s]

COLORS = ["#2a78d6", "#eb6834", "#1baf7a"]  # module 1, 2, 3 of each arm
INK, INK2, GRID = "#0b0b0b", "#52514e", "#e4e3df"


def find_latest_record(folder):
    """Return the most recent record file (.csv first, else .txt) in `folder`."""
    files = glob.glob(os.path.join(folder, "*-record.csv"))
    if not files:
        files = glob.glob(os.path.join(folder, "*-record.txt"))
    if not files:
        raise FileNotFoundError(f"No record found in {folder}. Press 'r' in the serial monitor to download the scope data.")
    return max(files, key=os.path.getmtime)


def read_record(path):
    """Read a ScopeMimicry record (.csv or .txt) as a DataFrame with one column per channel."""
    if path.endswith(".csv"):
        df = pd.read_csv(path, index_col=0)
    else:
        # .txt format: channel names, then the index of the last acquired point, then one float per line
        with open(path, "r") as f:
            lines = f.readlines()
        names = [n for n in lines.pop(0).replace("#", "").strip().split(",") if n]
        final_idx = int(lines.pop(0).replace("#", "").strip())
        datas = np.array([float(x) for x in lines if x.strip()]).reshape(-1, len(names))
        # The scope buffer is circular: put the oldest point first
        datas = np.roll(datas, -(final_idx + 1), axis=0)
        df = pd.DataFrame(datas, columns=names)
    df.columns = [c.strip() for c in df.columns]
    return df


def plot_arm(df, time, arm):
    """Plot N and the gate signals of one arm ('u' = upper, 'l' = lower)."""
    n_name = f"N_{arm}"
    gates = [f"g_{arm}_{k}" for k in (1, 2, 3)]
    missing = [c for c in [n_name] + gates if c not in df.columns]
    if missing:
        raise KeyError(f"Channels {missing} not found in the record (found: {list(df.columns)})")

    arm_name = "Upper arm" if arm == "u" else "Lower arm"
    first_module = 1 if arm == "u" else 4
    fig, axs = plt.subplots(4, 1, figsize=(10, 7), sharex=True)

    axs[0].step(time, df[n_name], where="post", color=INK, lw=2)
    axs[0].set_ylabel(f"{n_name}\n(modules)", color=INK2)
    axs[0].set_yticks([0, 1, 2, 3])
    axs[0].set_ylim(-0.3, 3.3)

    for k, (gate, color) in enumerate(zip(gates, COLORS)):
        ax = axs[k + 1]
        ax.step(time, df[gate], where="post", color=color, lw=2)
        ax.set_ylabel(f"{gate}\n(M{first_module + k})", color=INK2)
        ax.set_yticks([0, 1])
        ax.set_yticklabels(["off", "on"])
        ax.set_ylim(-0.2, 1.2)

    for ax in axs:
        ax.grid(color=GRID, lw=0.8)
        ax.tick_params(colors=INK2)
        for side in ("top", "right"):
            ax.spines[side].set_visible(False)
    axs[-1].set_xlabel("Time [s]", color=INK2)
    fig.suptitle(f"CVB logic test - {arm_name}: number of modules to connect and gate signals",
                 x=0.06, ha="left", fontsize=12, fontweight="bold", color=INK)
    fig.tight_layout()
    return fig


def main():
    parser = argparse.ArgumentParser(description="Plot the ScopeMimicry data of the CVB control logic example.")
    parser.add_argument("record", nargs="?", help="record file (.csv or .txt); default: latest record in src/Data_records")
    parser.add_argument("--folder", default=os.path.join(".", "src", "Data_records"), help="folder searched for the latest record")
    parser.add_argument("--dt", type=float, default=DEFAULT_DT, help="time between two scope points in s (default 0.01)")
    parser.add_argument("--no-show", action="store_true", help="only save the figures, do not open them")
    args = parser.parse_args()

    path = args.record or find_latest_record(args.folder)
    print(f"Plotting {path}")
    df = read_record(path)
    time = np.arange(len(df)) * args.dt

    base = os.path.splitext(path)[0]
    for arm, suffix in (("u", "upper_arm"), ("l", "lower_arm")):
        fig = plot_arm(df, time, arm)
        out = f"{base}_{suffix}.png"
        fig.savefig(out, dpi=150, facecolor="white")
        print(f"Saved {out}")

    if not args.no_show:
        plt.show()


if __name__ == "__main__":
    main()
