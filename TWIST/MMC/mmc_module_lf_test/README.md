# MMC module - Low-frequency test

## Objectives and context

The goal of this tutorial is to test a individual MMC module operation in all its states in both low frequency. For that, we used the circuit structure below. R_{dec} resistance is connected in parallel to the module in order to emulated negative currents discharging the module capacitor.

<img width="950" height="337" alt="image" src="https://github.com/user-attachments/assets/767e4a0e-1f4f-4deb-a81a-e97e42c9170a" />

The board serves as a HB module of the MMC by programming its second leg (LEG2) to be deactivated, having the following circuit. Observe that the HIGH terminals are open (Vhigh to GND).

<img width="930" height="292" alt="image" src="https://github.com/user-attachments/assets/a64b8101-93ff-4cf3-aa89-9a7673162536" />

The tutorial will consist of putting the module M1 in Disconnected, Connected and Blocked states according to the correspondence table below:

<img width="1352" height="261" alt="image" src="https://github.com/user-attachments/assets/7d381cb1-fced-4388-8abd-d87edbf79fba" />

The module state is controlled using the board first leg (LEG1) switches Q1 and Q2 command (sw_{Q1} and sw_{Q2}):
Connected state:
- $sw_{Q1}=1$ (close Q1 MOSFET) and $sw_{Q2}=0$ (open Q2 MOSFET)
- The module is considered to be connected to the circuit, contributing to the stack with its capacitor voltage and the current pass through its capacitor.
Disconnected state:
- $sw_{Q1}=0$ (open Q1 MOSFET) and $sw_{Q2}=1$ (close Q2 MOSFET)
- The module is considered to be bypassed from the circuit having null voltage output and the current does not pass through its capacitor.
Blocked state:
- $sw_{Q1}=0$ (open Q1 MOSFET) and $sw_{Q2}=0$ (open Q2 MOSFET)
- Since both switches gate commands are to be turned off, only the MOSFETs anti-parallel diodes conducts. Which diodes conducts will depend only if current sign is positive or negative.

All these different states are illustrated in the figure below according to the current sign:

<img width="1132" height="487" alt="image" src="https://github.com/user-attachments/assets/652a469a-8b22-483e-88c5-8a05e72940e8" />

First, the individual module test is performed in low-frequency and then in high-frequency. The figure below shows the low-frequency test sequence. The user must have a programmable DC power supply that TURN ON the udc power supply to start the sequence first part (positive current) and TURN OFF udc after 0.45 s to start the sequence second part (negative current).

<img width="945" height="294" alt="image" src="https://github.com/user-attachments/assets/db20a0f7-6d7f-4b38-8f06-c4141e7eacdc" />


## Required Hardware list:
- 1 TWIST boards with SPIN
- 1 Programmable DC Power Supply (48 V, 2 A)
- Protection diode (at least 50 V)
- 2 resistors of 15 $\Omega$
- PC 64-bits (windows or linux)
- Oscilloscope
- Current probe
- 2 BNC adaptors to differential voltage measurement or differential probes

## Required Software list:
- Git
- Visual Studio Code with PlatformIO

!!! If not done yet you should have done the initial tutorial on how to setup VScode for OwnTech usage.

## Get tutorial code from VScode

First, we need to load the Single module test code from the OwnTech example repository version in https://github.com/analuhaas/examples 
1.	In VScode, open the folder where you previously cloned OwnTech’s github.
2.	In platformio.ini file, substitute the owntech_examples variable link by https://github.com/analuhaas/examples.git 
3.	Go to platform.io icon <img width="35" height="36" alt="image" src="https://github.com/user-attachments/assets/d438a26a-e041-48ff-955b-d3b3a01fe9c4" />, go to Examples Twist under the Project Tasks tab and click on “Single MMC module – LF test (modified)”.

<img width="456" height="358" alt="image" src="https://github.com/user-attachments/assets/8f169340-3e0a-42a9-a91e-f5a9272749ab" />

4.	Now you have the good code to execute the tutorial in your VScode.

## Hardware setup
5.	Recommended for MMC use: Configure the board to feed the 6 V auxiliary input externally with feeder completely disconnected from the electrical circuit of the board.
   In the TWIST board, it is possible to feed the auxiliary circuit of the board by 3 different ways:
  	* Using the feeder to provide the 6 V auxiliary input
   * Feeding the 6 V auxiliary input externally
   * Feeding the 6 V auxiliary input externally with feeder completely disconnected from the electrical circuit of the board.

The last one is preferable to MMC use cause the board feeder is not yet adapted to MMC charging phase. To do that:

- Connect an External Auxiliary DC Power Supply to the TWIST via the 6 V and DGND PINs. Make sure the External Auxiliary DC Power Supply is configured to deliver 6 V with limiting current above 0.5 A. Make sure this power supply OUTPUT IS OFF.

<img width="400" height="330" alt="image" src="https://github.com/user-attachments/assets/a94ab8b6-9aa4-456f-a9c2-988328fe550e" />

- Open the jumper JP5001 of the board by cutting the jumper connections using a cutter with an appropriate camera to see the area

<img width="788" height="354" alt="image" src="https://github.com/user-attachments/assets/4ea8605a-20df-4adf-a9dc-0c007057e639" />

6.	Recommended for MMC use: Disconnect the electrolytical capacitor of both Low1 and Low2 terminals adding the following code lines in the setup_routine() function

 shield.power.disconnectCapacitor(LEG1);
 shield.power.disconnectCapacitor(LEG2);

 In the TWIST board electrical circuit, we see that Clow1 and Clow2 low-side capacitors are divided in a electrolytical part that is removable by Q5 and Q6 and a ceramic part. It is preferable that these capacitors are disconnected since they are not traditionally present in a Half-Bridge module circuit.

 <img width="656" height="319" alt="image" src="https://github.com/user-attachments/assets/8ab85781-59dc-4b52-b1c4-91194d41f7ef" />

 7.	Connect the board with the power supply, the R and Rdec resistances, oscilloscope and PC as in the image: 
<img width="1067" height="573" alt="image" src="https://github.com/user-attachments/assets/41cd7576-07c1-47ae-9e40-0f0423eca4d8" />

 After making the connections and connecting the PC to board the USB-C cable, you should have something like this:

<img width="1327" height="1613" alt="module_test_hardware_ready" src="https://github.com/user-attachments/assets/527becd1-1492-48b0-b913-91a34c17fefd" />

8.	Configure your Oscilloscope with PicoScope (tutorial using PicoScope 4444):

a.	Make sure it acquires the following measurements:

i.	Module capacitor voltage (Vhigh to GND).

ii.	Module current.

iii.	DC power supply voltage.

  	b.	Use a 500 ms/div configuration with sampling rate to have 5 µs sampling.
   
 <img width="241" height="464" alt="image" src="https://github.com/user-attachments/assets/5cf1f3be-8028-41da-925b-4e4f1639ef43" />
 
  c.	Configure it to trigger when the power supply voltage rises up to 2 V.
  
 <img width="368" height="542" alt="image" src="https://github.com/user-attachments/assets/0f122b93-632a-467f-9edb-b42d6651bf6e" />

## Executing the LF module test

9.	Get the low-frequency test code using “Single MMC module – LF test” button.
10.	Build <img width="25" height="27" alt="image" src="https://github.com/user-attachments/assets/028ecb2b-5d25-4383-8ece-c51cb804c64f" /> and Upload <img width="34" height="25" alt="image" src="https://github.com/user-attachments/assets/5af20606-e19c-46fd-870b-326c0d855a70" /> the code main.cpp into the board.
11.	Make sure the External Auxiliary DC Power Supply is configured to deliver 6 V with limiting current above 0.5 A. TURN ON the External Auxiliary DC Power Supply output.
The boards starts in IDLE mode (equal to module blocked state).
12.	Configure your power supply to do the following sequence: turn on and turn off after 0.45 s.
13.	TURN ON the main power supply output. This will start the sequence first part.
14.	Verify if the DC power supply was turned off around 0.45 s as expected. After you verify both sequence parts were executed, the test is concluded.
15.	TURN OFF the External Auxiliary DC Power Supply output.
16.	Save your oscilloscope data.

## Expected results

In the low-frequency sequence, the module expected behavior is as shown in the figure below.

<img width="945" height="332" alt="image" src="https://github.com/user-attachments/assets/e1471572-c0a0-47c8-8ca2-d2f85616b41b" />

If you perform the low-frequency test with external 6V and feeder disconnected from the circuit, you should expect an experimental result like this:

<img width="945" height="432" alt="image" src="https://github.com/user-attachments/assets/7b0683e3-7c8e-42be-b1d0-32fb877f25ea" />

The use of external 6 V auxiliary input changes module discharge behavior while in disconnected state due to feeder behavior:

<img width="945" height="432" alt="image" src="https://github.com/user-attachments/assets/417b057a-e1a1-41cd-87e9-9574e940bc9f" />

If you zoom between 0.35 s and 0.38 s where the module commutes with $u_{dc}$ ON:

<img width="945" height="432" alt="image" src="https://github.com/user-attachments/assets/6266f19a-f249-4fea-8d58-2a6d034e123e" />
