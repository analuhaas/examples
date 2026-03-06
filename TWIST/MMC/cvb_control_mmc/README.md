# Capacitor Voltage Balancing (CVB) algorithm logic

## Objectives

The goal of this tutorial is test the logic of the Capacitor Voltage Balancing (CVB) algorithm. The CVB algorithm is used in the MMC in order to create a preference order for the connection of modules into the stack.

 <img width="945" height="341" alt="image" src="https://github.com/user-attachments/assets/848341be-ee60-488e-bf3f-5d69d4b51945" />


This algorithm is extremely important since the module main element, the capacitor charges and discharges according to the current as seen in the figure:

 <img width="874" height="653" alt="image" src="https://github.com/user-attachments/assets/9842a805-6e27-46b2-83e5-f82e0fea8e54" />


Therefore, if the connection order is random, the voltage of the modules in one stack are going to have different values every time, which is not good for the converter operation. So, we use the CVB algorithm with the objective of maintaining the capacitors voltages of all modules within one stack in the same mean level.

The algorithm works as follows:
- The algorithm receives the number of modules to be connected N from the modulation block.
- It verifies if this number is different from 0.
- If the stack current i_{arm} is positive, the capacitors are going to be charged. Then, the modules with smaller voltage are connected such that their voltage will increase.
- If the stack current i_{arm} is negative, the capacitors are going to be discharged. Then, the modules with higher voltage are connected such that their voltage will decrease.

 <img width="747" height="494" alt="image" src="https://github.com/user-attachments/assets/af6605a5-e1e9-4a78-9fa3-3e852cb7cb5a" />

To do this, we need to first sort the modules according to their capacitor voltages and then pass through the algorithm that will generate the switching signals indicating if they are connected or not.

## Required Hardware

•	1 TWIST boards with SPIN

•	PC 64-bits (windows or linux)

## Required Software

•	Git

•	Visual Studio Code with PlatformIO 

## Get tutorial code from VScode

First, we need to load the CVB control code from the OwnTech example repository version in https://github.com/analuhaas/examples 
1.	In VScode, open the folder where you previously cloned OwnTech’s github.
2.	In platformio.ini file, substitute the owntech_examples variable link by https://github.com/analuhaas/examples.git 
3.	Go to platform.io icon  , go to Examples Twist under the Project Tasks tab and click on “CVB control”.

 <img width="200" height="157" alt="image" src="https://github.com/user-attachments/assets/c04f9cfb-801b-4e77-8fa3-d0fb59f6939e" />

5.	Now you have the good code to execute the tutorial in your VScode.

#### Main code structure

The `main.cpp` structure is shown in the image below.

![Code structure](Image/main_structure_FirmwareCVBgate.png)

The code structure is as follows:
- On the top of the code some initialization functions and variables definition take place
- **Setup Routine** - Calls functions that set the hardware and software
- **Communication Task** - Handles the keyboard communication and decides which `MODE` is activated
- **Application Task** - Activates the LED and prints data on the serial port according to the `MODE`
- **Critical Task** - Handles the control according to the `MODE`, effectively implements the CVB algorithm and gate assignement

The tasks are executed following the diagram below. 


![Timing diagram](Image/timing_diagram.png)


- **Communication Task** - Is awaken regularly to verify any keyboard activity
- **Application Task** - This task is woken once its suspend is finished 
- **Critical Task** - This task is driven by the HRTIM count interrupt, where it counts a number of HRTIM switching frequency periods. In this case 100us, or 20 periods of the TWIST board 200kHz switching frequency set by default.

### Bubble sorting

In this Capacitor Voltage Balacing example, we use bubble sorting. Bubble sort is one of the simplest sorting algorithms. The idea is to go through a set of elements several times, and with each pass, move the largest element in the sequence to the top.

<img width="556" height="358" alt="image" src="https://github.com/user-attachments/assets/84637f4c-6745-43b7-a3ab-05356552347f" />

## Hardware setup

6.	Connect the board with the PC using a USB to USB-C cable.

![wiring diagram](Image/cvb_wiring.jpeg)

## Executing the tutorial

7.	Optional: Change the parameters to have a more specific test:
* Switching frequency (variable sw_period - default 10000 equivalent to 1 Hz)
* Scope configuration
* Number of modules
* Current sense (variables i_upper_arm and i_lower_arm)
* Capacitor voltages (variables modules_capacitor_voltages_upper_arm and modules_capacitor_voltages_lower_arm)
* Implement and test your own sorting or CVB algorithm!

8.	Build <img width="26" height="26" alt="image" src="https://github.com/user-attachments/assets/eae7af84-438e-41ef-bff5-c00379819461" />
and Upload <img width="40" height="31" alt="image" src="https://github.com/user-attachments/assets/679225ab-f391-44d6-bb93-0cf1048914a5" />
the code main.cpp into the board
9.	Open the serial monitor using the <img width="27" height="27" alt="image" src="https://github.com/user-attachments/assets/cf3dfa70-be47-4386-b2a8-71fbbea037ca" />
icon while the USB-C cable is connected with the board, as shown in the hardware setup.
10.	Press h to display the help menu. The board starts in IDLE mode.
11.	Press p to switch to POWER mode and the CVB starts to choose the connected and disconnected modules according to the connection sequence.
12.	Press a to trigger data acquisition of Scopemimicry scope.
13.	Press r when you think you have sufficient data to record.
14.	Press i to switch to IDLE mode and stop the CVB operation.
15.	Data will be recorded in src>>Data_records folder.

## Expected results

By plotting the recorded data, you will have results like these:

<img width="945" height="464" alt="image" src="https://github.com/user-attachments/assets/da48d5be-c71b-4549-b053-d2c462ca6da7" />

These results were obtained using modules_capacitor_voltages_upper_arm[3] = {3.0,5.0,4.0} and i_upper_arm = 1 ($i\geq 0$) for the upper stack to test the algorithm, so the preference of connection should be $g_{u1}\rightarrow g_{u3}\rightarrow g_{u2}$.

You can plot the same for the lower stack using modules_capacitor_voltages_lower_arm[3] = {3.0,5.0,4.0} and i_lower_arm = -1 ($i < 0$), so the preference of connection should be $g_{u2}\rightarrow g_{u3}\rightarrow g_{u1}$.

<img width="945" height="464" alt="image" src="https://github.com/user-attachments/assets/80f85447-3dbe-4379-af49-38e6f84796ae" />

To fully understand the CVB algorithm, you can play with:

•	Scope configuration

•	Connection sequence steps frequency

•	Number of modules

•	Current signal

•	Capacitor voltages
