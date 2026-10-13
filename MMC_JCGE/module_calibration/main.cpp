/*
 * Copyright (c) 2021-present LAAS-CNRS
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
 * @brief  This example demonstrates how to calibrate a MMC HB module based on
 *         the TWIST/SPIN board manually, using the serial monitor. LEG1 or
 *         LEG2 is driven at duty cycle 1 and the raw sensor values are
 *         printed on the serial monitor.
 *
 * @author Clément Foucher <clement.foucher@laas.fr>
 * @author Luiz Villa <luiz.villa@laas.fr>
 * @author Ayoub Farah Hassan <ayoub.farah-hassan@laas.fr>
 */

/*--------------Zephyr---------------------------------------- */
#include <zephyr/console/console.h>

/*--------------OWNTECH APIs---------------------------------- */
#include "SpinAPI.h"
#include "ShieldAPI.h"
#include "TaskAPI.h"

/*--------------SETUP FUNCTIONS DECLARATION------------------- */
/* Setups the hardware and software of the system */
void setup_routine();

/*--------------LOOP FUNCTIONS DECLARATION-------------------- */
/* Code to be executed in the slow communication task */
void loop_communication_task();
/* Code to be executed in the background task */
void loop_application_task();
/* Code to be executed in real time in the critical task */
void loop_critical_task();

/*--------------USER VARIABLES DECLARATIONS------------------- */

/* [us] period of the control task */
static uint32_t control_task_period = 100;
/* [bool] state of the PWM (ctrl task) */
static bool pwm_enable = false;

uint8_t received_serial_char;

/* Measure variables */

static float32_t V1_low_value;
static float32_t V2_low_value;
static float32_t I1_low_value;
static float32_t I2_low_value;
static float32_t I_high;
static float32_t V_high;

static float32_t temp_1_value;
static float32_t temp_2_value;

/* Temporary storage fore measured value (ctrl task) */
static float meas_data;

/*--------------------------------------------------------------- */

/* LIST OF POSSIBLE MODES FOR THE OWNTECH CONVERTER */
enum serial_interface_menu_mode
{
    IDLEMODE = 0,
    POWERMODE1 = 1, /* LEG1 at duty cycle 1.0: calibration of the LOW1 side */
    POWERMODE2 = 2  /* LEG2 at duty cycle 1.0: calibration of the LOW2 side */
};

/* Current converter mode, updated by the critical task */
uint8_t mode = IDLEMODE;
/* Mode asked from the serial monitor */
uint8_t mode_asked = IDLEMODE;

/*--------------SETUP FUNCTIONS------------------------------- */

/**
 * This is the setup routine.
 * Here the setup :
 *  - Initializes the power shield in Buck mode
 *  - Initializes the power shield sensors
 *  - Initializes the PID controller
 *  - Spawns three tasks.
 */
void setup_routine()
{
    /* Buck voltage mode */
    shield.power.initBuck(ALL);

    shield.sensors.enableDefaultTwistSensors();

    shield.power.disconnectCapacitor(LEG1);
    shield.power.disconnectCapacitor(LEG2);
    
    /* Enable switch control with max and min duty cycle*/
    shield.power.setDutyCycleMax(ALL,1.0);
    shield.power.setDutyCycleMin(ALL,0.0);

    shield.sensors.setConversionParametersLinear(V_HIGH, 1,0);
    shield.sensors.setConversionParametersLinear(V1_LOW, 1,0);
    shield.sensors.setConversionParametersLinear(V2_LOW, 1,0);
    shield.sensors.setConversionParametersLinear(I1_LOW, 1,0);
    shield.sensors.setConversionParametersLinear(I2_LOW, 1,0);
    shield.sensors.setConversionParametersLinear(I_HIGH, 1,0);

    /* Then declare tasks */
    uint32_t app_task_number = task.createBackground(loop_application_task);
    uint32_t com_task_number = task.createBackground(loop_communication_task);
    task.createCritical(loop_critical_task, 100);

    /* Finally, start tasks */
    task.startBackground(app_task_number);
    task.startBackground(com_task_number);
    task.startCritical();
}

/*--------------LOOP FUNCTIONS-------------------------------- */

/**
 * This tasks implements a minimalistic USB serial interface to control
 * the buck converter.
 */
void loop_communication_task()
{
    received_serial_char = console_getchar();
    switch (received_serial_char)
    {
    case 'h':
        /*----------SERIAL INTERFACE MENU----------------------- */
        printk(" ________________________________________ \n"
               "|     ---- MENU sensor calibration ----  |\n"
               "|     press i : idle mode                |\n"
               "|     press 1 : LEG1 at duty cycle 1     |\n"
               "|     press 2 : LEG2 at duty cycle 1     |\n"
               "|________________________________________|\n\n");
        /*------------------------------------------------------ */
        break;
    case 'i':
        printk("idle mode\n");
        mode_asked = IDLEMODE;
        break;
    case '1':
        printk("power mode 1: LEG1 at duty cycle 1\n");
        mode_asked = POWERMODE1;
        break;
    case '2':
        printk("power mode 2: LEG2 at duty cycle 1\n");
        mode_asked = POWERMODE2;
        break;
    default:
        break;
    }
}

/**
 * This is the code loop of the background task
 * This task mostly logs back measurements to the USB serial interface.
 */
void loop_application_task()
{
    if (mode == IDLEMODE)
    {
        spin.led.turnOff();
    }
    else
    {
        spin.led.turnOn();

        shield.sensors.triggerTwistTempMeas(TEMP_SENSOR_1);
        shield.sensors.triggerTwistTempMeas(TEMP_SENSOR_2);

        meas_data = shield.sensors.getLatestValue(TEMP_SENSOR_1);
        if (meas_data != NO_VALUE) temp_1_value = meas_data;

        meas_data = shield.sensors.getLatestValue(TEMP_SENSOR_2);
        if (meas_data != NO_VALUE) temp_2_value = meas_data;


        /* Raw values: V_HIGH:V1_LOW:V2_LOW:I_HIGH:I1_LOW:I2_LOW */
        printk("%.3f:", (double)V_high);
        printk("%.3f:", (double)V1_low_value);
        printk("%.3f:", (double)V2_low_value);
        printk("%.3f:", (double)I_high);
        printk("%.3f:", (double)I1_low_value);
        printk("%.3f\n", (double)I2_low_value);
    }
    task.suspendBackgroundMs(100);
}

/**
 * This is the code loop of the critical task
 * This task runs at 10kHz.
 *  - It retrieves sensors values
 *  - It runs the PID controller
 *  - It update the PWM signals
 */
void loop_critical_task()
{
    meas_data = shield.sensors.getLatestValue(I1_LOW);
    if (meas_data != NO_VALUE) I1_low_value = meas_data;

    meas_data = shield.sensors.getLatestValue(V1_LOW);
    if (meas_data != NO_VALUE) V1_low_value = meas_data;

    meas_data = shield.sensors.getLatestValue(V2_LOW);
    if (meas_data != NO_VALUE) V2_low_value = meas_data;

    meas_data = shield.sensors.getLatestValue(I2_LOW);
    if (meas_data != NO_VALUE) I2_low_value = meas_data;

    meas_data = shield.sensors.getLatestValue(I_HIGH);
    if (meas_data != NO_VALUE) I_high = meas_data;

    meas_data = shield.sensors.getLatestValue(V_HIGH);
    if (meas_data != NO_VALUE) V_high = meas_data;

    mode = mode_asked;

    if (mode == IDLEMODE)
    {
        if (pwm_enable == true)
        {
            shield.power.stop(ALL);
        }
        pwm_enable = false;
    }
    else if (mode == POWERMODE1)
    {
        shield.power.setDutyCycle(LEG1, 1.0);
        if (!pwm_enable)
        {
            pwm_enable = true;
            shield.power.start(LEG1);
            shield.power.stop(LEG2);
        }
    }
    else if (mode == POWERMODE2)
    {
        shield.power.setDutyCycle(LEG2, 1.0);
        if (!pwm_enable)
        {
            pwm_enable = true;
            shield.power.start(LEG2);
            shield.power.stop(LEG1);
        }
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