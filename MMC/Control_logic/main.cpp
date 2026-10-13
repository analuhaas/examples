/*
 * Copyright (c) 2026-present LAAS-CNRS
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 2.1 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

/**
 * @brief  This example tests the logic of the Capacitor Voltage Balancing (CVB) algorithm of a MMC phase (upper and lower arms) on a single SPIN board.
 *         The number of modules to connect comes from Nearest Level Modulation (NLM), and the capacitor voltages and arm currents are fixed test values.
 *         This research was funded in whole by the French National Research Agency (ANR) under the project CARROTS "ANR-24-CE05-0920-01".
 *
 * @author Ayoub Farah Hassan <ayoub.farah-hassan@laas.fr>
 * @author Ana Luiza Haas Bezerra <ana-luiza.haas-bezerra@centralesupelec.fr>
 * @author Zaid Jabbar <zaid.jabbar@grenoble-inp.fr>
 * @author Luiz Villa <luiz.villa@laas.fr>
 * @author Jean Alinei <jean.alinei@owntech.org>
 * @author Noemi Lanciotti <noemi.lanciotti@centralesupelec.fr>
 * @author Loïc Quéval <loic.queval@centralesupelec.fr>
 */

/* --------------OWNTECH APIs---------------------------------- */
#include "SpinAPI.h"
#include "TaskAPI.h"
#include "ShieldAPI.h"

/*--------------OWNTECH Libraries----------------------------- */
#include "trigo.h"
#include "arm_math_types.h"
#include <ScopeMimicry.h>

/*-- Zephyr includes --*/
#include "zephyr/console/console.h"

/* -------------- GENERAL MMC DEFINITIONS -------------------- */
/* --------------- To be changed by user --------------------- */

static const float f0 = 0.1F; //[Hz] Frequency of the sinusoidal reference for open-loop control (low to follow the steps on the terminal and the LED)
static const uint8_t total_number_of_modules_arm = 3; //[-] Number of modules per arm

/* Test values replacing the measurements of a real MMC */
static const float32_t test_capacitor_voltages_upper_arm[total_number_of_modules_arm] = {3.0F, 5.0F, 4.0F}; //[V] Upper arm modules capacitor voltages (M1, M2, M3)
static const float32_t test_capacitor_voltages_lower_arm[total_number_of_modules_arm] = {3.0F, 5.0F, 4.0F}; //[V] Lower arm modules capacitor voltages (M4, M5, M6)
static float32_t i_upper_arm = 1.0F; //[A] Upper arm current - positive: capacitors charge
static float32_t i_lower_arm = -1.0F; //[A] Lower arm current - negative: capacitors discharge

/* --------------SETUP FUNCTIONS DECLARATION------------------- */

/* Setups the hardware and software of the system */
void setup_routine();

/* --------------LOOP FUNCTIONS DECLARATION-------------------- */

/* Code to be executed in the background task - sets up board LED and prints the CVB output in the terminal */
void loop_background_task();
/* Code to be executed in real time in the critical task - executes the NLM and the CVB algorithm */
void loop_critical_task();
/* Code to be executed in the communication task - serves to send command to board via PC using USB-C cable */
void loop_communication_task();

/* --------------USER VARIABLES DECLARATIONS------------------- */

uint8_t received_serial_char; // Variable to store the received character from the serial interface

/* --------------- LIST OF POSSIBLE BOARD MODES ------------------*/
enum serial_interface_menu_mode
{
    IDLEMODE = 0, // CVB logic stopped
    POWERMODE = 1, // NLM and CVB logic running
};

serial_interface_menu_mode mode = IDLEMODE;

/* --------------- Firmware CVB variables ------------------*/

/* [us] period of the control task (=critical task) */
static constexpr uint32_t control_task_period = 100; // µs
static float32_t Ts = control_task_period * 1e-6F; // s
/* [bool] state of the PWM (ctrl task) */
static bool pwm_enable = false;

/* Scope variables */
static bool enable_acq; // Sets trigger moment if true
static const uint16_t NB_DATAS = 1024; // Number of data acquired
static ScopeMimicry scope(NB_DATAS, 8); // Scope configuration with 8 channels
static bool is_downloading; // Records data if true
static uint32_t scope_timer = 0;
static uint32_t scope_period = 100; // scope acquire data every t = scope_period (100) * critical_task_period (100 µs) = 10 ms

/* CVB variables */

static uint8_t index_list[10] = {0,1,2,3,4,5,6,7,8,9}; // Modules general indexes array
static float32_t number_of_connected_modules_upper_arm; // Stores number of modules connected in the upper arm (NLM output)
static float32_t number_of_connected_modules_lower_arm; // Stores number of modules connected in the lower arm (NLM output)
static float32_t number_of_connected_modules_upper_arm_past = 0.0F; // Stores number of modules connected in the upper arm in t-1
static float32_t number_of_connected_modules_lower_arm_past = 0.0F; // Stores number of modules connected in the lower arm in t-1
static float32_t modules_capacitor_voltages_upper_arm[total_number_of_modules_arm]; // Upper arm modules capacitor voltages, copied from the test values before each sorting
static uint8_t modules_indexes_upper_arm[total_number_of_modules_arm]; // Upper arm modules indexes to be sorted with the capacitor voltage vector
static float32_t modules_capacitor_voltages_lower_arm[total_number_of_modules_arm]; // Lower arm modules capacitor voltages, copied from the test values before each sorting
static uint8_t modules_indexes_lower_arm[total_number_of_modules_arm]; // Lower arm modules indexes to be sorted with the capacitor voltage vector

/* Gate logic */
uint8_t g_u[total_number_of_modules_arm]; // Gate signals of the upper modules
uint8_t g_l[total_number_of_modules_arm]; // Gate signals of the lower modules
static float32_t g_u_1; // Gate signal M1 - Used for gate signal acquisition by scopemimicry
static float32_t g_u_2; // Gate signal M2 - Used for gate signal acquisition by scopemimicry
static float32_t g_u_3; // Gate signal M3 - Used for gate signal acquisition by scopemimicry
static float32_t g_l_1; // Gate signal M4 - Used for gate signal acquisition by scopemimicry
static float32_t g_l_2; // Gate signal M5 - Used for gate signal acquisition by scopemimicry
static float32_t g_l_3; // Gate signal M6 - Used for gate signal acquisition by scopemimicry

/* NLM */
static float32_t m = 1; // Modulation amplitude
static float32_t a = 1; // Modulation dc part
static float32_t angle;
static const float w0 = 2 * PI * f0; // Angular frequency

static float32_t modulation_signal_upper; //[pu] Modulation output upper voltage
static float32_t modulation_signal_lower; //[pu] Modulation output lower voltage

/* --------------SETUP FUNCTIONS------------------------------- */

/* Trigger function for scope manager */
bool a_trigger()
{
    return enable_acq;
}

/* Records scope data */
void dump_scope_datas(ScopeMimicry &scope)
{
    uint8_t *buffer = scope.get_buffer();
    /* We divide by 4 (4 bytes per float data) */
    uint16_t buffer_size = scope.get_buffer_size() >> 2;
    printk("begin record\n");
    printk("#");
    for (uint16_t k = 0; k < scope.get_nb_channel(); k++)
    {
        printk("%s,", scope.get_channel_name(k));
    }
    printk("\n");
    printk("# %d\n", scope.get_final_idx());
    for (uint16_t k = 0; k < buffer_size; k++)
    {
        printk("%08x\n", *((uint32_t *)buffer + k));
        task.suspendBackgroundUs(100);
    }
    printk("end record\n");
}

/**
 * This is the setup routine.
 * It is used to call functions that will initialize your spin, power shields
 * and tasks.
 */
void setup_routine()
{
    shield.power.initBuck(ALL);
    shield.sensors.enableDefaultTwistSensors();

    /* Configures scopemimicry measured variables */
    scope.connectChannel(number_of_connected_modules_upper_arm, "N_u");
    scope.connectChannel(number_of_connected_modules_lower_arm, "N_l");
    scope.connectChannel(g_u_1, "g_u_1");
    scope.connectChannel(g_u_2, "g_u_2");
    scope.connectChannel(g_u_3, "g_u_3");
    scope.connectChannel(g_l_1, "g_l_1");
    scope.connectChannel(g_l_2, "g_l_2");
    scope.connectChannel(g_l_3, "g_l_3");
    scope.set_trigger(&a_trigger);
    scope.set_delay(0.0F);
    scope.start();

    /* Declare tasks */
    uint32_t background_task_number = task.createBackground(loop_background_task);
    uint32_t communication_task_number = task.createBackground(loop_communication_task);
    task.createCritical(loop_critical_task, control_task_period);

    /* Finally, start tasks */
    task.startBackground(background_task_number);
    task.startBackground(communication_task_number);
    task.startCritical();
}

/* --------------LOOP FUNCTIONS-------------------------------- */

/**
 * This is the communication task.
 * It is used to send to the board via the computer the desired mode
 * IDLE (i) = stop the CVB logic or POWER (p) = run the NLM and the CVB logic.
 *
 * It also sends data acquisition start command (a) and scope data retrieve commands (r).
 */
void loop_communication_task()
{
    received_serial_char = console_getchar();

    switch (received_serial_char)
    {
    case 'h':
        /*----------SERIAL INTERFACE MENU----------------------- */
        printk(" ________________________________________ \n"
               "|     ---- MENU CVB logic test ----      |\n"
               "|     press i : idle mode                |\n"
               "|     press p : power mode               |\n"
               "|     press r : record data              |\n"
               "|     press a : toggle enable_acq var    |\n"
               "|________________________________________|\n\n");
        /*------------------------------------------------------ */
        break;
    case 'i':
        printk("idle mode\n");
        mode = IDLEMODE;
        break;
    case 'p':
        printk("power mode\n");
        mode = POWERMODE;
        break;
    case 'r':
        is_downloading = true;
        break;
    case 'a':
        enable_acq = !(enable_acq);
        break;
    default:
        break;
    }
}

/**
 * This is the code loop of the background task
 * It runs perpetually. Here a `suspendBackgroundMs` is used to pause during
 * 1000ms between each LED toggles.
 * Hence we expect the LED to blink each second.
 *
 * In POWER mode, it also prints the NLM and CVB outputs in the terminal:
 * N_u:N_l:g_u_1:g_u_2:g_u_3:g_l_1:g_l_2:g_l_3
 */
void loop_background_task()
{
    if (mode == IDLEMODE)
    {
        spin.led.turnOff();
        if (is_downloading)
        {
            dump_scope_datas(scope);
            is_downloading = false;
        }
    }
    if (mode == POWERMODE)
    {
        spin.led.toggle();
        printk("%.0f:", (double)number_of_connected_modules_upper_arm);
        printk("%.0f:", (double)number_of_connected_modules_lower_arm);
        printk("%.0f:", (double)g_u_1);
        printk("%.0f:", (double)g_u_2);
        printk("%.0f:", (double)g_u_3);
        printk("%.0f:", (double)g_l_1);
        printk("%.0f:", (double)g_l_2);
        printk("%.0f\n", (double)g_l_3);
    }

    task.suspendBackgroundMs(1000);
}

/**
 * @brief Capacitor Voltage Balancing (CVB) algorithm - Determine which modules connect/disconnect on upper arm.
 *
 * @param ...
 * @return Gate signals for upper arm.
 */
void sorting_upper_arm()
{
    /* Reset upper modules indexes every time the function is used */
    memcpy(modules_indexes_upper_arm, index_list, total_number_of_modules_arm);
    
    /* Sorts upper modules indexes according to capacitor voltage in ascending order (lower to higher voltage) */
    uint8_t counter_loops_sorting = 0;
    while(counter_loops_sorting < total_number_of_modules_arm + 1){ 
            /* Bubble sorting technique - simple */
            for(uint8_t counter = 0; counter < total_number_of_modules_arm-1; counter++)
            {
                if(modules_capacitor_voltages_upper_arm[counter] > modules_capacitor_voltages_upper_arm[counter + 1])
                {
                    float32_t temp = modules_capacitor_voltages_upper_arm[counter];
                    modules_capacitor_voltages_upper_arm[counter] = modules_capacitor_voltages_upper_arm[counter + 1];
                    modules_capacitor_voltages_upper_arm[counter + 1] = temp;
                    float32_t temp2 = modules_indexes_upper_arm[counter];
                    modules_indexes_upper_arm[counter] = modules_indexes_upper_arm[counter + 1];
                    modules_indexes_upper_arm[counter + 1] = temp2;
                }
            }

            counter_loops_sorting++;
        }

    /* Choses the modules to connect to the upper arm according to capacitor voltages and arm current */
    for(uint8_t counter = 0; counter < total_number_of_modules_arm; counter++)
        {
            /* Positive arm current */
            // Connect modules with smallest capacitor voltages
            // Disconnect modules with highest capacitor voltages

            if(i_upper_arm>=0)
            {
                uint8_t index_smallest_voltage_capacitor_upper_arm = modules_indexes_upper_arm[counter];
                if(counter < number_of_connected_modules_upper_arm)
                {
                    g_u[index_smallest_voltage_capacitor_upper_arm] = 1;
                }
                else{
                    g_u[index_smallest_voltage_capacitor_upper_arm] = 0;
                }
            }

            /* Negative arm current */
            // Connect modules with highest capacitor voltages
            // Disconnect modules with smallest capacitor voltages
            if(i_upper_arm<0)
            {
                uint8_t higher_index = total_number_of_modules_arm-1-counter;
                uint8_t index_highest_voltage_capacitor_upper_arm = modules_indexes_upper_arm[higher_index];
                if(counter < number_of_connected_modules_upper_arm)
                {
                    g_u[index_highest_voltage_capacitor_upper_arm] = 1;
                }
                else{
                    g_u[index_highest_voltage_capacitor_upper_arm] = 0;
                }
            }   
        }

}

/**
 * @brief Capacitor Voltage Balancing (CVB) algorithm - Determine which modules connect/disconnect on lower arm.
 *
 * @param ...
 * @return Gate signals for lower arm.
 */
void sorting_lower_arm()
{
    /* Reset lower modules indexes every time the function is used */
    memcpy(modules_indexes_lower_arm, index_list, total_number_of_modules_arm);
    
    /* Sorts lower modules indexes according to capacitor voltage in ascending order (lower to higher voltage) */
    uint8_t counter_loops_sorting = 0;
    while(counter_loops_sorting < total_number_of_modules_arm + 1){ 
            /* Bubble sorting technique - simple */
            for(uint8_t counter = 0; counter < total_number_of_modules_arm-1; counter++)
            {
                if(modules_capacitor_voltages_lower_arm[counter] > modules_capacitor_voltages_lower_arm[counter + 1])
                {
                    float32_t temp = modules_capacitor_voltages_lower_arm[counter];
                    modules_capacitor_voltages_lower_arm[counter] = modules_capacitor_voltages_lower_arm[counter + 1];
                    modules_capacitor_voltages_lower_arm[counter + 1] = temp;
                    float32_t temp2 = modules_indexes_lower_arm[counter];
                    modules_indexes_lower_arm[counter] = modules_indexes_lower_arm[counter + 1];
                    modules_indexes_lower_arm[counter + 1] = temp2;
                }
            }

            counter_loops_sorting++;
        }

    /* Choses the modules to connect to the lower arm according to capacitor voltages and arm current */
    for(uint8_t counter = 0; counter < total_number_of_modules_arm; counter++)
        {
            /* Positive arm current */
            // Connect modules with smallest capacitor voltages
            // Disconnect modules with highest capacitor voltages

            if(i_lower_arm>=0)
            {
                uint8_t index_smallest_voltage_capacitor_lower_arm = modules_indexes_lower_arm[counter];
                if(counter < number_of_connected_modules_lower_arm)
                {
                    g_l[index_smallest_voltage_capacitor_lower_arm] = 1;
                }
                else{
                    g_l[index_smallest_voltage_capacitor_lower_arm] = 0;
                }
            }

            /* Negative arm current */
            // Connect modules with highest capacitor voltages
            // Disconnect modules with smallest capacitor voltages
            if(i_lower_arm<0)
            {
                uint8_t higher_index = total_number_of_modules_arm-1-counter;
                uint8_t index_highest_voltage_capacitor_lower_arm = modules_indexes_lower_arm[higher_index];
                if(counter < number_of_connected_modules_lower_arm)
                {
                    g_l[index_highest_voltage_capacitor_lower_arm] = 1;
                }
                else{
                    g_l[index_highest_voltage_capacitor_lower_arm] = 0;
                }
            }   
        }

}

/**
 * This is the code loop of the critical task
 * It is executed every control_task_period (100 micro-seconds) defined in the
 * setup_routine function.
 *
 * In the critical task, we implement the MMC control algorithms that will
 * run in Real Time.
 */
void loop_critical_task()
{
    if (mode == POWERMODE)
    {
        /* Modulation signal generation in open-loop */
        angle += w0 * Ts;
        angle = ot_modulo_2pi(angle);
        m = 1;
        modulation_signal_upper = (a + m * ot_sin(angle)) / (2.0);
        modulation_signal_lower = (a - m * ot_sin(angle)) / (2.0);

        /* Number of modules N_on to be connected on the arm according to modulation signal by Nearest Level Modulation (NLM)  */
        number_of_connected_modules_upper_arm = round(total_number_of_modules_arm*modulation_signal_upper);
        number_of_connected_modules_lower_arm = round(total_number_of_modules_arm*modulation_signal_lower);

        /* Modules choice with Capacitor Voltage Balancing (CVB) sorting */
        // Executed only when N_on changes in one of the arms
        if (number_of_connected_modules_upper_arm != number_of_connected_modules_upper_arm_past
            || number_of_connected_modules_lower_arm != number_of_connected_modules_lower_arm_past){

            /* Updating capacitor voltages measurements (test values) */
            memcpy(modules_capacitor_voltages_upper_arm, test_capacitor_voltages_upper_arm, total_number_of_modules_arm * sizeof(float32_t));
            memcpy(modules_capacitor_voltages_lower_arm, test_capacitor_voltages_lower_arm, total_number_of_modules_arm * sizeof(float32_t));

            /* Executes the CVB algorithm, chosing which modules to connect */
            sorting_upper_arm();
            sorting_lower_arm();
            number_of_connected_modules_upper_arm_past = number_of_connected_modules_upper_arm;
            number_of_connected_modules_lower_arm_past = number_of_connected_modules_lower_arm;
        }

        /* Scope data acquisition */
        g_u_1 = (float)g_u[0];  // recuperate for scope acquisition
        g_u_2 = (float)g_u[1];  // recuperate for scope acquisition
        g_u_3 = (float)g_u[2];  // recuperate for scope acquisition
        g_l_1 = (float)g_l[0];  // recuperate for scope acquisition
        g_l_2 = (float)g_l[1];  // recuperate for scope acquisition
        g_l_3 = (float)g_l[2];  // recuperate for scope acquisition

        if (scope_timer == scope_period)
        {
            scope.acquire();
            scope_timer = 0;
        }
        scope_timer++;
    }
    else if (mode == IDLEMODE)
    {
        if (pwm_enable == true)
        {
            shield.power.stop(ALL);
        }
        pwm_enable = false;
    }
}

/**
 * This is the main function of this example
 * This function is generic and does not need editing.
 */
int main(void)
{
    setup_routine();

    return 0;
}
