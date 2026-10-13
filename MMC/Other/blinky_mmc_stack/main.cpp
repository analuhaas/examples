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
 * @brief  This example shows how to a MMC arm works by blinking the onboard LED of the Spin board of the arm modules.
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
#include "CommunicationAPI.h"

/*--------------OWNTECH Libraries----------------------------- */
#include "arm_math_types.h"
#include <ScopeMimicry.h>

/*-- Zephyr includes --*/
#include "zephyr/console/console.h"

/* -----Boards roles identification, LEAD = MMC_LEAD---------- */
#define MMC_LEAD 0
#define MMC_M1 1
#define MMC_M2 2
#define MMC_M3 3
#define MMC_M4 4
#define MMC_M5 5
#define MMC_M6 6

/* --------- BOARD IDENTIFICATION functions ------------------ */
/**
 * @brief This function is considering a byte called 'cmd'
 *        which can turn on or off signals.
 *        The signals are identified by their 'id'.
 *        The value 'val' is used to set the signal:
 *        - if val is true, the signal is set to 1
 *        - if val is false, the signal is set to 0
 */
#define SET_SIGNAL(cmd, id, val)   \
    do                             \
    {                              \
        if (val)                   \
            (cmd) |= (1 << (id));  \
        else                       \
            (cmd) &= ~(1 << (id)); \
    } while (0)

/**
 * @brief This function is to get the turn on/off state of a signal
 *        identified by its 'id' from a byte called 'cmd'.
 */
#define GET_SIGNAL(cmd, id) (((cmd) >> (id)) & 0x01)

/* --------------SETUP FUNCTIONS DECLARATION------------------- */

/* Setups the hardware and software of the system */
void setup_routine();

/* --------------LOOP FUNCTIONS DECLARATION-------------------- */

/* Code to be executed in the background task - only sets up boards LEDs */
void loop_background_task();
/* Code to be executed in real time in the critical task - executes all LEAD and MODULES control logics */
void loop_critical_task();
/* Code to be executed in the communication task - serves to send command to board via PC using USB-C cable */
void loop_communication_task();

/* --------------USER VARIABLES DECLARATIONS------------------- */

/* Define module_ID depending on the ID of the board - To be changed by the user */
uint8_t module_ID = MMC_M1; // The ID of the module, can be set to MMC_LEAD or any other SMx

static uint8_t module_comand; // The command the followers needs to apply
static uint8_t module_command_past; // The command the followers applied in t-1 (last critical task)
static bool change_state_command = false; // Flag to change the state of the command
static bool send_idle = false;            // Flag to send idle command from master to followers

/**
 * This is a structure that defines the frame
 * that will be sent and received through the RS485 communication.
 * command is a byte that contains the state of the signals
 * Capacitor_Voltage is the voltage of the capacitor
 * ID is the ID of the module
 */
struct MMC_frame
{
    uint8_t command;
    float32_t Capacitor_Voltage;
    uint8_t status;
    uint8_t ID;
} __packed;

typedef MMC_frame MMC_frame_t;
static MMC_frame_t dataTX_mmc;
static MMC_frame_t dataRX_mmc;

float32_t MMC_capacitor_voltage[6];

uint8_t buffer_tx[7];
uint8_t buffer_rx[7];

float32_t MMC_voltage = 0.0f;

uint32_t counter_timer = 0;
uint32_t counter_receive = 0;

uint8_t received_serial_char; // Variable to store the received character from the serial interface
int8_t CommTask_num;

/* --------------- LIST OF POSSIBLE BOARD MODES ------------------*/
enum serial_interface_menu_mode
{
    IDLEMODE = 0, // Related to blocked state
    POWERMODE = 1, // Related to connected/disconnected state
};

serial_interface_menu_mode mode = IDLEMODE;

void loop_communication_task(); // Code to be executed in the communication task

/* --------------- Firmware and control variables ------------------*/

/* [us] period of the control task (=critical task) */
static uint32_t control_task_period = 100; // µs
/* [bool] state of the PWM (ctrl task) */
static bool pwm_enable = false;

/* Measure variables */

static float32_t V1_low_value;
static float32_t V2_low_value;
static float32_t I1_low_value;
static float32_t I2_low_value;
static float32_t I_high;
static float32_t V_high;

static float32_t temp_1_value;
static float32_t temp_2_value;

/* Temporary storage for measured value (ctrl task) */
static float meas_data;

/* Scope variables */
static bool enable_acq; // Sets trigger moment if true
static const uint16_t NB_DATAS = 2048; // Number of data acquired
static ScopeMimicry scope(NB_DATAS, 5); // Scope configuration with 5 channels
static bool is_downloading; // Records data if true

/* SM switching variables */

static float32_t number_of_connected_submodules_upper_arm; // Stores number of modules connected in the upper arm
static float32_t number_of_connected_submodules_lower_arm; // Stores number of modules connected in the lower arm
static uint8_t seq_u[6] = {1, 2, 3, 2, 1, 0}; // Connection sequence for upper arm
static uint8_t seq_l[6] = {2, 1, 0, 1, 2, 3}; // Connection sequence for lower arm
static uint8_t counter_seq = 0;
static uint32_t sw_timer = 0;
static uint32_t scope_timer = 0;
static uint32_t sw_period = 1000; // 1 Hz = 1 s period to transition to next connection sequence value;
static uint32_t scope_period = 1; // scope acquire data every t = scope_period * critical_task_period (100 µs) s;

/* Gate logic */
uint8_t g[3] = {0, 0, 0}; // Gate signals to be sent to the modules
static float32_t g_u_1; // Gate signal M1 - Used for gate signal acquisition by scopemimicry
static float32_t g_u_2; // Gate signal M2 - Used for gate signal acquisition by scopemimicry
static float32_t g_u_3; // Gate signal M3 - Used for gate signal acquisition by scopemimicry
static float32_t g_l_1; // Gate signal M4 - Used for gate signal acquisition by scopemimicry
static float32_t g_l_2; // Gate signal M5 - Used for gate signal acquisition by scopemimicry
static float32_t g_l_3; // Gate signal M6 - Used for gate signal acquisition by scopemimicry

/* --------------SETUP FUNCTIONS------------------------------- */

/* Function to control the LEDs in the low level */
void config_led_LL()
{
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_5, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_5, LL_GPIO_SPEED_FREQ_VERY_HIGH);
    LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_5, LL_GPIO_OUTPUT_PUSHPULL);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_5, LL_GPIO_PULL_NO);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
}

inline void Led_turnON_LL()
{
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
}

inline void Led_turnOFF_LL()
{
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
}

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

/* RS-485 reception_function: executed when a message is received */
void reception_function(void)
{
    dataRX_mmc = *(MMC_frame_t *)buffer_rx;

    if (module_ID == MMC_LEAD)
    {
        MMC_capacitor_voltage[dataRX_mmc.ID - 1] = dataRX_mmc.Capacitor_Voltage;
    }

    else
    {
        if (dataRX_mmc.ID == MMC_LEAD)
        {
            /* retrievig command from lead message*/
            module_comand = GET_SIGNAL(dataRX_mmc.command, module_ID);
            /* retrieving status */
            if (dataRX_mmc.status == 1)
            {
                mode = POWERMODE;
            }
            else
            {
                mode = IDLEMODE;
            }
        }

        /* The board following the ID of the one who sent will start sending
            the next message */
        if ((dataRX_mmc.ID == module_ID - 1))
        {
            dataTX_mmc = dataRX_mmc; // Copy the received data to the transmission data
            dataTX_mmc.ID = module_ID;
            dataTX_mmc.Capacitor_Voltage = MMC_voltage; /* TODO :uncomment when we get the voltage */
            if (mode == POWERMODE)
            {
                memcpy(buffer_tx, &dataTX_mmc, sizeof(dataTX_mmc));
                communication.rs485.startTransmission();
            }
        }
    }
    counter_receive++;
}

/**
 * This is the setup routine.
 * It is used to call functions that will initialize your spin, power shields
 * and tasks.
 */
void setup_routine()
{

    /* Configure the LED pin in Low Level */
    config_led_LL();

    /* Buck mode */
    shield.power.initBuck(ALL);

    /* Declare task */
    uint32_t background_task_number =
        task.createBackground(loop_background_task);
    task.createCritical(loop_critical_task, 100);

    shield.sensors.enableDefaultTwistSensors();

    /* Finally, start tasks */
    task.startBackground(background_task_number);
    task.startCritical();
    CommTask_num = task.createBackground(loop_communication_task);
    task.startBackground(CommTask_num);

    communication.rs485.configure(buffer_tx, buffer_rx, sizeof(buffer_rx),
                                  reception_function,
                                  SPEED_20M); // custom configuration for RS485
                                              /* Configure scope channels, what measurements do you want to acquire? */
    if (module_ID == MMC_LEAD)
    {
        /* Defines lead's clock as reference for communication synchorinization */
        communication.sync.initMaster();

        /* Configures scopemimicry measured variables */
        scope.connectChannel(number_of_connected_submodules_upper_arm, "N_u");
        scope.connectChannel(number_of_connected_submodules_lower_arm, "N_l");
        scope.connectChannel(g_u_1, "g_u_1");
        scope.connectChannel(g_u_2, "g_u_2");
        scope.connectChannel(g_u_3, "g_u_3");
        scope.set_trigger(&a_trigger);
        scope.set_delay(0.0F);
        scope.start();
    }
    else{
        /* Defines module as follower for communication synchorinization */
        communication.sync.initSlave();
    }
}

/* --------------LOOP FUNCTIONS-------------------------------- */

/**
 * This is the communication task.
 * It is used to send to the board via the computer the desired mode
 * IDLE (i) = block all modules or POWER (p) = operate Blinky MMC arm.
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
               "|     ---- MENU buck voltage mode ----   |\n"
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
        send_idle = false; // Set the flag to send idle command to false 
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
 * 2000ms between each LED toggles.
 * Hence we expect the LED to blink each 2 seconds.
 */
void loop_background_task()
{
    if (module_ID == MMC_LEAD)
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
            printk("%1.f:", number_of_connected_submodules_upper_arm);
            printk("%1.f:", number_of_connected_submodules_lower_arm);
            printk("%u:", counter_seq);
            printk("%u:", sw_timer);
            printk("%u:", g_u_1);
            printk("%u:", g_u_2);
            printk("%u:", g_u_3);
            printk("\n");
        }
    }

    task.suspendBackgroundMs(2000);
}

/**
 * This is the code loop of the critical task
 * It is executed every 100 micro-seconds defined in the setup_software
 * function.
 *
 * In the critical task, we implement the MMC control algorithms that will
 * run in Real Time.
 */
void loop_critical_task()
{
    if (mode == POWERMODE)
    {
        if (module_ID == MMC_LEAD) //CONTROL INSIDE LEAD - Connection seq generation + sets modules connection order
        {
            /* Connection sequence of "triangular" format generation */
            if (sw_timer == sw_period)
            {
                if (counter_seq >= 6)
                {
                    counter_seq = 0;
                }

                /* Number of modules N_on to be connected on the arm according to generated sequence */
                number_of_connected_submodules_upper_arm = (float)seq_u[counter_seq];
                number_of_connected_submodules_lower_arm = (float)seq_l[counter_seq];
                counter_seq++;
                sw_timer = 0;
            }

             /* Modules choice with preference order M1 > M2 > M3 */
            if (number_of_connected_submodules_upper_arm == 0)
            {
                g[0] = 0;
                g[1] = 0;
                g[2] = 0;
            }
            if (number_of_connected_submodules_upper_arm == 1)
            {
                g[0] = 1;
                g[1] = 0;
                g[2] = 0;
            }
            if (number_of_connected_submodules_upper_arm == 2)
            {
                g[0] = 1;
                g[1] = 1;
                g[2] = 0;
            }
            if (number_of_connected_submodules_upper_arm == 3)
            {
                g[0] = 1;
                g[1] = 1;
                g[2] = 1;
            }

            /* Scope data acquisition */
            g_u_1 = (float)g[0]; // recuperate for scope
            g_u_2 = (float)g[1]; // recuperate for scope
            g_u_3 = (float)g[2]; // recuperate for scope

            if (scope_timer == scope_period)
            {
                scope.acquire();
                scope_timer = 0;
            }
            sw_timer++;
            scope_timer++;

            /* Modules state assignment according to preference order to be sent to the modules */
            SET_SIGNAL(dataTX_mmc.command, MMC_M1, g[0]);
            SET_SIGNAL(dataTX_mmc.command, MMC_M2, g[1]);
            SET_SIGNAL(dataTX_mmc.command, MMC_M3, g[2]);

            /* Fills all other communication trame spaces */
            dataTX_mmc.ID = module_ID;
            memcpy(buffer_tx, &dataTX_mmc, sizeof(dataTX_mmc));
            dataTX_mmc.status = 1;

            /* LEAD communicates to MODULES */
            communication.rs485.startTransmission();
        }
        else //CONTROL INSIDE MODULE - own LED switching only
        {
            /* Verifies if module received command changed */
            if (module_comand != module_command_past)
            {
                change_state_command = true; // Set the flag to change the state
            }

            //If command is 1, module changes to connected state and LED turns ON
            if (module_comand)
            {
                if (change_state_command)
                {
                    Led_turnON_LL(); // Turn ON SPIN LED
                    change_state_command = false; // Reset the flag
                }
            }
            //If command is 0, module changes to disconnected state and LED turns OFF
            else
            {
                if (change_state_command)
                {
                    Led_turnOFF_LL(); // Turn OFF SPIN LED
                    change_state_command = false; // Reset the flag
                }
            }
        }
        module_command_past = module_comand; // Update the past command
    }
    else if (mode == IDLEMODE)
    {
        /* Made  such that the LEAD send IDLE flag only once to all modules */
        if (!send_idle)
        {
            dataTX_mmc.ID = module_ID;
            dataTX_mmc.status = 0;
            memcpy(buffer_tx, &dataTX_mmc, sizeof(dataTX_mmc));
            communication.rs485.startTransmission();
            send_idle = true; // Set the flag to send idle command to true, meaning that idle mode is active
        }
    }
    counter_timer++;
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