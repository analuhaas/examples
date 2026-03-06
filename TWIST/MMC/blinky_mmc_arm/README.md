# Blinky MMC arm

The goal of this tutorial is to do a first MMC arm without using power with the following structure:

![Blinky MMC idea](Image/blinky_mmc_idea.png)

The main board serves as the central controller in the classical structure of the MMC. It will generate a connection sequence going from 0 to 3 that indicates how many modules should be connected. Then, it choses which modules to connect according to the preset preference order M1 → M2 → M3.

Then, the main board sends the connection commands g1, g2 and g3 via the Ethernet cables to the M1, M2 and M3 boards. If the board receives the command to be connected, it turns ON its SPIN board LED. If the board receives the command to be disconnected, it turns OFF its SPIN board LED.

Attention: Are you ready to start ?
    Before you can run this example, you must have successfully gone through the [VScode getting started](https://github.com/analuhaas/MMC/blob/tutorials_updates/MMC_documentation/Vscode%20for%20OwnTech%20configuration.md).  

## Required Hardware

•	4 TWIST boards with SPIN

•	DC Power Supply (48 V, 2 A)

•	3 Ethernet cables (RJ45)

•	Cables to connect with the power supply

•	PC 64-bits (windows or linux)

## Required Software

•	Git

•	Visual Studio Code with PlatformIO 

## Get tutorial code from VScode

First, we need to load the Blinky MMC stack code from the OwnTech example repository version in https://github.com/analuhaas/examples 
1.	In VScode, open the folder where you previously cloned OwnTech’s github.
2.	In platformio.ini file, substitute the owntech_examples variable link by https://github.com/analuhaas/examples.git 
3.	Go to platform.io icon  , go to Examples Twist under the Project Tasks tab and click on “Blinky MMC arm”.

 <img width="200" height="157" alt="image" src="https://github.com/user-attachments/assets/8a7caae0-2a27-446f-ae00-dbf916f47d0d" />

4.	Now you have the good code to execute the tutorial in your VScode.

#### Main code structure

The `main.cpp` structure is shown in the image below.

![Code structure](Image/main_structure_Blinky_MMC.png)

The code structure is as follows:
- On the top of the code some initialization functions and variables definition take place
- **Setup Routine** - Calls functions that set the hardware and software, and configures the RS485 communication
- **Communication Task** - Handles the keyboard communication with the #LEAD and decides which `MODE` is activated
- **Application Task** - Activates the #LEAD LED and prints data on the serial port according to the `MODE`
- **Critical Task** - Handles the control according to the `MODE`, effectively implements the connection sequence and data transmission from #LEAD to all #SMx

After the #LEAD starts the transmission, the reception_function is used in the receiver to store the information and continue the transmission sequence to the next board.

The tasks are executed following the diagram below. 

![Timing diagram](Image/timing_diagram.png)


- **Communication Task** - Is awaken regularly to verify any keyboard activity
- **Application Task** - This task is woken once its suspend is finished 
- **Critical Task** - This task is driven by the HRTIM count interrupt, where it counts a number of HRTIM switching frequency periods. In this case 100us, or 20 periods of the TWIST board 200kHz switching frequency set by default.

#### Connection sequence

The connection sequence has a specific frequency set by the variable sw_period and the critical task period.

![Connection sequence](Image/width_step.png)

The value of the connection sequence signal corresponds to the number of modules to insert. Then, using the preset connection order, we have:

![Connection sequence expected result](Image/connection_results_schematic.png)

#### Communication structure

In the critical task, when POWER mode is activated, data transmission is first started sending the data structure dataTX_mmc from #LEAD to all #SMx.

The data structure sent is defined as:
```
dataTX_mmc or dataRX_mmc
└─ command
└─ capacitor_voltage
└─ status
└─ ID
```
In which:
- command = command to be inserted (1) or bypassed (0) sent to the module
- capacitor_voltage = stores the measured capacitor voltage on the #SMx
- status = flag corresponding to POWER (1) or IDLE (0) mode
- ID = identification of the board as #LEAD or #SMx

After the #LEAD starts the transmission, the reception_function is used in the receiver to store the information and continue the transmission sequence to the next board with sequence LEAD → M3 → M2 → M1

## Hardware setup

5.	Connect the 4 TWIST/SPIN board with the DC power supply as in the image below.
6.	Connect the 4 TWIST/SPIN boards together via ethernet cables to enable communication as in the image below.
7.	Connect the Main TWIST/SPIN board with the PC as in the image below using a USB to USB-C cable.

![wiring diagram](Image/wiring_Blinky_MMC_arm.png)

After making the connections and connecting the PC to the main with the USB-C cable, you should have something like this:
<img width="655" height="491" alt="image" src="https://github.com/user-attachments/assets/f00e40d0-d8cd-466b-9989-639ec222e86c" />

<img width="758" height="85" alt="image" src="https://github.com/user-attachments/assets/c70de4ea-1be1-4e34-bf1d-82dbfb5c7316" />

## Executing the tutorial

8.	Optional: Change switching frequency using sw_period variable (default 10000 equivalent to 1 Hz)
9.	Repeat for all 4 boards:
    - Build <img width="26" height="26" alt="image" src="https://github.com/user-attachments/assets/b76afa22-1370-44ea-b8a6-de654ba3a3ac" />
and Upload <img width="40" height="31" alt="image" src="https://github.com/user-attachments/assets/dfe091d8-802c-462d-b31b-6450e9c9158b" /> main.ccp code to the board changing the variable module_ID to MMC_LEAD, MMC_SM1, MMC_SM2 or MMC_SM3 according to the board function.

```
/* --------------USER VARIABLES DECLARATIONS------------------- */

/* TODO : Define module_ID depending on the ID of the board */
uint8_t module_ID = MMC_SM1; // The ID of the module, can be set to MMC_LEAD or any other SMx
```

10.	Open the serial monitor using the   icon with USB-C cable connected with the master, as shown in the hardware setup.
11.	Press h to display the help menu.
12.	Turn on the power supply around 15 V to power the 4 SPIN boards. The main and M1, M2 and M3 boards starts in IDLE mode.
13.	Press p to switch to POWER mode and the modules LEDs will start to light up and off according to the connection sequence.
14.	Press i to switch to IDLE mode and stop the stack operation.
15.	Turn off the power supply.

## Expected results during POWER mode

https://github.com/user-attachments/assets/b48dbae1-9d3b-4fd4-9842-21abe6c78a97
