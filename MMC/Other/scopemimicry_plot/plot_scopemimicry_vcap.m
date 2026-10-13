%% Plot ScopeMimicry capacitor voltages of the MMC stack and phase tests

close all, clear all, clc

%% Settings
test = "stack"; % "stack" or "phase"

directory_mimicry = 'Data_records'; % Folder with the ScopeMimicry csv files
file_mimicry = "2026-10-12_10-00-00-record.csv"; % Name of the ScopeMimicry csv file

T_critical_task = 200e-6; % [s] control_task_period of the code
scope_period = 1; % scope_period of the code

t_lim = [0, 0.2]; % [s] Time window of the plots

%% ScopeMimicry data
data_mimicry = readtable(fullfile(directory_mimicry, file_mimicry), "VariableNamingRule", "preserve");

t_mimicry = data_mimicry{:, 1}*T_critical_task*scope_period;

if test == "stack"
    modules_upper = 1:5;
    modules_lower = [];
else
    modules_upper = 1:5;
    modules_lower = 6:10;
end

%% Plot figure
n_plots = 3 + (test == "phase");
figure(1), clf

% Upper arm capacitor voltages
subplot(n_plots,1,1), box on, grid on, hold on
plot_vcap(t_mimicry, data_mimicry, modules_upper)
xlim(t_lim)
ylabel('$v_{cap}$ [V]','Interpreter','latex')
if test == "phase", title('Upper arm'), end

% Lower arm capacitor voltages
if test == "phase"
    subplot(n_plots,1,2), box on, grid on, hold on
    plot_vcap(t_mimicry, data_mimicry, modules_lower)
    xlim(t_lim)
    ylabel('$v_{cap}$ [V]','Interpreter','latex')
    title('Lower arm')
end

% Stack current
subplot(n_plots,1,n_plots-1), box on, grid on, hold on

plot(t_mimicry,data_mimicry.i_u,'-','LineWidth',1);
names = {'$i_u$'};
if test == "phase"
    plot(t_mimicry,data_mimicry.i_l,'-','LineWidth',1);
    names{end+1} = '$i_l$';
end

legend(names,'Interpreter','latex'), legend boxon
xlim(t_lim)
ylabel('$i_{stack}$ [A]','Interpreter','latex')

% Number of connected modules
subplot(n_plots,1,n_plots), box on, grid on, hold on

stairs(t_mimicry,data_mimicry.N_u,'-','LineWidth',1);
names = {'$N_u$'};
if test == "phase"
    stairs(t_mimicry,data_mimicry.N_l,'-','LineWidth',1);
    names{end+1} = '$N_l$';
end
legend(names,'Interpreter','latex'), legend boxon
ylabel('$N_{on}$','Interpreter','latex')

xlim(t_lim)
xlabel('$time$ [s]','Interpreter','latex')

%% Functions
function plot_vcap(t, data, modules)
    names = {};
    for k = modules
        plot(t, data.("v_c_" + k), '-', 'LineWidth', 1);
        names{end+1} = sprintf('$v_{cap}^{M%d}$', k);
    end
    legend(names,'Interpreter','latex'), legend boxon
end
