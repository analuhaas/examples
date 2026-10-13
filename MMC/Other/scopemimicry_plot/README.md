# Plot the ScopeMimicry capacitor voltages

This folder contains a MATLAB code and a Python code to plot the capacitor voltages recorded by the ScopeMimicry scope of the lead board in the [MMC stack tutorials](../../Educational_prototype/stack_with_cvb/README.md) (with and without CVB) and in the [MMC phase tutorial](../../Educational_prototype/phase_NLM_with_cvb/README.md). Both codes do the same plot:

| File | Language |
|---|---|
| [plot_scopemimicry_vcap.m](plot_scopemimicry_vcap.m) | MATLAB |
| [plot_scopemimicry_vcap.py](plot_scopemimicry_vcap.py) | Python |

The capacitor voltages of the modules are measured by each module and sent to the lead board through the RS-485 communication, so ScopeMimicry gives all the capacitor voltages at the same time, without using one oscilloscope channel per module.

## Data recorded by ScopeMimicry

The lead board records one point every critical task period (200 µs), so the 1028 points of the scope cover about 0.2 s. The channels recorded by each code are:

| Code | Channels |
|---|---|
| Stack with CVB / stack without CVB | `N_u`, `v_c_1` to `v_c_5`, `i_u`, `g_u_1` to `g_u_5`, `i_u_filtered` |
| Phase | `N_u`, `N_l`, `v_c_1` to `v_c_10`, `i_u`, `i_l`, `i_u_filtered` |

- `v_c_k`: capacitor voltage of module Mk. In the phase, M1 to M5 are the upper arm and M6 to M10 the lower arm.
- `i_u`, `i_l`: upper and lower arm current, measured by the first module of each arm.
- `N_u`, `N_l`: number of modules to connect in the upper and lower arm (NLM output).
- `g_u_k`: gate signal of the upper arm module Mk (stack codes only).

The codes read the columns by their name, so they still work if you add other channels to the scope.

## Getting the data

1. During the stack or phase test, press `a` in the serial monitor to start the ScopeMimicry acquisition.
2. Press `i` to go back to IDLE mode, then `r` to download the data.
3. The OwnTech serial monitor saves the data in the `src/Data_records` folder of your project, as `<date>_<time>-record.csv` (and `.txt` and `.png`).
4. Copy the `.csv` file to a `Data_records` folder next to the plot code, or change `directory_mimicry` in the settings.

## Settings

The settings are at the top of both codes:

| Setting | Default | Meaning |
|---|---|---|
| `test` | `"stack"` | `"stack"` or `"phase"`: plots 5 or 10 capacitor voltages |
| `directory_mimicry` | `Data_records` | Folder with the ScopeMimicry csv files |
| `file_mimicry` | – | Name of the ScopeMimicry csv file |
| `T_critical_task` | 200e-6 s | `control_task_period` of the code |
| `scope_period` | 1 | `scope_period` of the code |
| `t_lim` | [0, 0.2] s | Time window of the plots |

## Running the codes

- **MATLAB:** open `plot_scopemimicry_vcap.m`, change the settings and click Run.
- **Python:** install the packages once, change the settings and run the code:

    ```bash
    pip install pandas matplotlib
    python plot_scopemimicry_vcap.py
    ```

The figure shows, from top to bottom:

1. the capacitor voltages of the upper arm modules (and of the lower arm modules for the phase),
2. the arm current `i_u` (and `i_l` for the phase),
3. the number of modules to connect `N_u` (and `N_l` for the phase).

