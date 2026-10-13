# Plot ScopeMimicry capacitor voltages of the MMC stack and phase tests

import os
import pandas as pd
import matplotlib.pyplot as plt

# Settings
test = "stack"  # "stack" or "phase"

directory_mimicry = "Data_records"  # Folder with the ScopeMimicry csv files
file_mimicry = "2026-10-12_10-00-00-record.csv"  # Name of the ScopeMimicry csv file

T_critical_task = 200e-6  # [s] control_task_period of the code
scope_period = 1  # scope_period of the code

t_lim = [0, 0.2]  # [s] Time window of the plots

# ScopeMimicry data
data_mimicry = pd.read_csv(os.path.join(directory_mimicry, file_mimicry))
data_mimicry.columns = [c.strip() for c in data_mimicry.columns]

t_mimicry = data_mimicry.iloc[:, 0] * T_critical_task * scope_period

modules_upper = [1, 2, 3, 4, 5]
modules_lower = [6, 7, 8, 9, 10] if test == "phase" else []


def plot_vcap(ax, modules):
    for k in modules:
        ax.plot(t_mimicry, data_mimicry[f"v_c_{k}"], "-", linewidth=1, label=f"$v_{{cap}}^{{M{k}}}$")
    ax.legend(loc="upper left", bbox_to_anchor=(1.0, 1.0))


# Plot figure
n_plots = 4 if test == "phase" else 3
fig, axs = plt.subplots(n_plots, 1, sharex=True, figsize=(9, 2.2 * n_plots))

# Upper arm capacitor voltages
plot_vcap(axs[0], modules_upper)
axs[0].set_ylabel("$v_{cap}$ [V]")
if test == "phase":
    axs[0].set_title("Upper arm")

    # Lower arm capacitor voltages
    plot_vcap(axs[1], modules_lower)
    axs[1].set_ylabel("$v_{cap}$ [V]")
    axs[1].set_title("Lower arm")

# Stack current
ax = axs[n_plots - 2]
ax.plot(t_mimicry, data_mimicry["i_u"], "-", linewidth=1, label="$i_u$")
if test == "phase":
    ax.plot(t_mimicry, data_mimicry["i_l"], "-", linewidth=1, label="$i_l$")
ax.set_ylabel("$i_{stack}$ [A]")
ax.legend(loc="upper left", bbox_to_anchor=(1.0, 1.0))

# Number of connected modules
ax = axs[n_plots - 1]
ax.step(t_mimicry, data_mimicry["N_u"], where="post", linewidth=1, label="$N_u$")
if test == "phase":
    ax.step(t_mimicry, data_mimicry["N_l"], where="post", linewidth=1, label="$N_l$")
ax.set_ylabel("$N_{on}$")
ax.legend(loc="upper left", bbox_to_anchor=(1.0, 1.0))
ax.set_xlabel("time [s]")

for ax in axs:
    ax.grid(True)
    ax.set_xlim(t_lim)

fig.tight_layout()
plt.show()
