/*serial_parser.ino
This file is free software; you can redistribute it and/or modify
it under the terms of either the GNU General Public License version 2
or the GNU Lesser General Public License version 2.1, both as
published by the Free Software Foundation.

Created by Wesley Kuegler (wes.kuegler@gmail.com) in Summer 2017 at NASA's Langley Research Center

This serial parser allows a user to operate the DACs on the MEDLI2 SSE Sensor Simulator board.
There are 11 4-channel DACs on the MEDLI2 SSE Sensor Simulator board, totaling 44 channels. Channels
38 and 44 are unconnected, but should still be set to some voltage during operation.

If using a serial monitor to operate the firmware, be sure to set the carriage return character to "Newline."

--------------------------------------------------------CHANNEL TEST HEADER MAPPINGS--------------------------------------------------------
CHANNEL-----TEST HEADER-----CHANNEL-----TEST HEADER-----CHANNEL-----TEST HEADER-----CHANNEL-----TEST HEADER-----CHANNEL-----TEST HEADER-----
0			TC1				10			TC11			20			TC21			30			P2				40			HF3
1			TC2				11			TC12			21			TC22			31			P3				41			HF4
2			TC3				12			TC13			22			TC23			32			P4				42			BSP
3			TC4				13			TC14			23			TC24			33			P5				43			UNCONNECTED
4			TC5				14			TC15			24			TC25			34			P6
5			TC6				15			TC16			25			TC26			35			P7
6			TC7				16			TC17			26			TC27			36			P8
7			TC8				17			TC18			27			TC28			37			UNCONNECTED
8			TC9				18			TC19			28			TC29			38			HF1
9			TC10			19			TC20			29			P1				39			HF2

----------------------------------------------------------IN THIS VERSION, V17.8.2----------------------------------------------------------
*added two coefficient arrays, stored in EEPROM. These will be used for error adjustment
*added functions for loading and saving the two arrays
*removed some extraneous commands that were only for testing purposes
*commented out "setscale" as it has stopped working

#####ISSUES#####
*two arrays of 44 floats each appears to push the Arduino just barely over its memory capacity, so storing the 88 values for the x- and y-
	coefficients of each channel might not be possible. the two coefficient arrays and their related functions are currently commented out
*"setscale" has inexplicably stopped working - no changes were made to the command

----------------------------------------------------------OLDER VERSION INFORMATION----------------------------------------------------------
-------------------------------------------------------------------V17.7.26----------------------------------------------------------
*removed the resistance and offset arrays, along with their associated functions and menu commands. Between offsets, resistances, and scaling
	factors, there is more that we want to store on the Arduino than we can fit in our 1KB of EEPROM. Instead, those values will be saved on
	the PC side, and matched to their Arduino by the Arduino's ID number
*added the functions loadID() and writeID(), which load the boardID from EEPROM and save it to EEPROM, respectively
*added boardID and setBoardID commands to the main menu

-------------------------------------------------------------------V17.7.21------------------------------------------------------------------
*printFloats() now prints out to 4 decimal places, instead of only 2
*the resistance value printing in "printResistance" also now prints to 4 decimal places

-------------------------------------------------------------------V17.7.6-------------------------------------------------------------------
*The firmware no longer waits for the serial port to open before beginning operation

-------------------------------------------------------------------V17.6.30------------------------------------------------------------------
*Made menu and command echo formatting clearer, and modified the parser to ignore end of line markers when the command string is empty
*Added "setsingle" command for setting the value of a single DAC channel
*Clarified bit order for "RTDMUX" command in the menu

-------------------------------------------------------------------V17.6.29------------------------------------------------------------------
*ISSUE RESOLVED - Dynamic allocation of memory for the various arrays isn't necessary. All array allocation has been changed to static, hard-
coded at 44 elements.

-------------------------------------------------------------------V17.6.28------------------------------------------------------------------
*Moved version information to versionInfo() function. Added a menu command that calls the versionInfo() function
*Added board ID number information to versionInfo()
*Added a menu command to set the 5 MUX bits for the RTD board
*The menu now echoes inputted commands back over the serial line
*Added reading and writing of resistor divider resistance values array to EEPROM
*Added menu command for setting resistance values
*Added menu command for printing out comma-separated resistance values over the serial line

#####ISSUES#####
*Dynamic allocation of array memory for resistances[] and offsets[] is causing an issue in which the "command" string is unable to be built.
-This issue is not present if static memory allocation is used, therefore the loop in setup() for allocating resistances[] is currently
commented out. resistances[] is hardcoded to 44 elements currently, and all resistance functionality is working otherwise as intended.

-------------------------------------------------------------------V17.6.27------------------------------------------------------------------
*The firmware is fully functional at the basic level: It can correctly set and clear the voltages on all DAC channels
*Added reading and writing of offset array to EEPROM
*Added version information to comments and main menu
*Added ASCII table of channel to test header mappings (top of file)
*/

//includes
#include <EEPROM.h>
#include "Keyboard.h"
#include <SPI.h>
#include "DAC57X4.h"
#include <time.h>
#include "string.h"

//globals
String version = "17.8.2";		//Year.Month.Day. BE SURE TO UPDATE THIS WHEN CHANGES ARE MADE
int boardID = 1;				//Identifies which main board copy this firmware is for
String command = "";	//hold inputted command from serial stream (in loop())
int DAC_count = 11;		//number of DACs
int DAC_channels = 4;	//number of channels per DAC
int total_channels = DAC_channels * DAC_count;	//number of channels across all 4-channel DACS
float voltages[44];		//array of voltages to be pushed to the DAC channels
//float xCoef[44];		//array of x-coefficients for error correction (to be stored in and loaded from EEPROM)
//float yCoef[44];		//array of y-coefficients for error correction (to be stored in and loaded from EEPROM)

DAC57X4 DAC = DAC57X4::DAC57X4(4, DAC_count, 5, A5, A3, A4);	/*create a new global DAC57X4 object.
														Each DAC in the chain has 4 channels. Use voltage setting 5 (+/- 10V). Pin A5 is the pin on
														the Arduino Micro that runs to the DAC SYNC line, A3 is the DAC CLR line, and A4 is the DAC
														SYNC line.*/

void setup()
{
	Serial.begin(9600);		//open serial port
	Serial.setTimeout(-1);	//no serial timeout
	
	//set pinmodes for RTD MUX pins 
	pinMode(1, OUTPUT);		//ARD_S1_A0 in layout
	pinMode(4, OUTPUT);		//ARD_S2_A0 in layout
	pinMode(5, OUTPUT);		//ARD_S2_A1 in layout
	pinMode(6, OUTPUT);		//ARD_S2_A2 in layout
	pinMode(7, OUTPUT);		//ARD_S2_A3 in layout
	
	pinMode(A2, OUTPUT);	//set pinMode for MUX select pin and LED pin
	pinMode(LED_BUILTIN, OUTPUT);

	digitalWrite(A2, HIGH);				//set the MUX select pin HIGH to switch to Arduino mode
	digitalWrite(LED_BUILTIN, HIGH);	//turn on the light so we can see if the sketch is running

	loadID();	//load the offsets from EEPROM into offsets array
	writeID();	//load the resistances from EEPROM into resistances array

	versionInfo();		
}

//main menu message
void menu() {
	Serial.print("\nEnter one of the following commands:\n");
	Serial.print(">SET - Set the voltages on all DAC channels.\n");
	Serial.print(">SETSINGLE - Set the voltage on a single DAC channel.\n");
	Serial.print(">RANDOM - Set the voltages on all DAC channels to random values between -10 and 10 volts.\n");
	Serial.print(">DEFAULT - Set the voltages on all DAC channels to .1 x their channel index.\n");
	Serial.print(">CLEAR - Set all DAC channel voltages to the clear value.\n");
	Serial.print(">BOARDID - Print the board ID.\n");
	Serial.print(">SETBOARDID - Set the board ID.\n");
	Serial.print(">RTDMUX - Set the 5 MUX bits on the RTD board.\n");
	Serial.print(">VERSION - Display version information.\n");
}

//version information
void versionInfo() {
	Serial.print("\nMEDLI2 SSE Sensor Simulator Firmware V"); Serial.print(version); Serial.print("\n");
	Serial.print("Running on MEDLI2 SSE Sensor Simulator Board "); Serial.print(boardID); Serial.print("\n");
}

//print out the DAC voltages array
void printFloats(float* floatArray) {
	for (int i = 0; i < total_channels; i++) {
		Serial.print("Channel ");
		Serial.print(i);		//begin with "Channel 0"
		Serial.print(": ");
		Serial.print(floatArray[i], 5);		//print the float to 4 decimal places
		Serial.print("V\n");
	}

	Serial.print("Done\n");
}

//push the voltage array out to the DAC channels
void DACset() {
	/*push all 44 voltages out to the DAC daisy chain. Begin with channel A of DAC 11,
	moving through each of the 4 channels (A,B,C,D) for each of the DACs (ex: 11,10,9,...1)*/
	for (int i = 0; i < DAC_channels; i++) {	//for each channel
												
		for (int j = DAC_count; j > 0; j--) {	//for each DAC
												
			DAC.pushDACvoltage(voltages[i + (4 * (j - 1))], i + 1);
		}
		DAC.SYNCdata();	//latch commands in all DAC input shift registers. 
		DAC.LoadDACs();	//send the load signal (LDAC) to the DACs
	}
}

//load the Arduino board ID from EEPROM
void loadID() {
	EEPROM.get(0, boardID);		//load the ID from address 0
}

//write the Arduino board ID to EEPROM
void writeID() {
	EEPROM.put(0, boardID);		//store the ID at address 0
}

////load xCoeff from EEPROM
//void loadX() {
//	for (int i = 0; i < total_channels; i++) {	//for every DAC channel
//		EEPROM.get(4 + (i * 4), xCoef[i]);		//load the coefficient from the next 4 byte step
//	}
//}
//
////write xCoeff to EEPROM
//void writeX() {
//	for (int i = 0; i < total_channels; i++) {	//for every DAC channel
//		EEPROM.put(4 + (i * 4), xCoef[i]);		//write the coefficient at the next 4 byte step
//	}
//}
//
////load yCoeff from EEPROM
//void loadY() {
//	for (int i = 0; i < total_channels; i++) {	//for every DAC channel
//		EEPROM.get(200 + (i * 4), yCoef[i]);		//load the coefficient from the next 4 byte step
//	}
//}
//
////write yCoeff to EEPROM
//void writeY() {
//	for (int i = 0; i < total_channels; i++) {	//for every DAC channel
//		EEPROM.put(200 + (i * 4), yCoef[i]);		//write the coefficient at the next 4 byte step
//	}
//}

//clear the voltage array to the lower rail
void clearVoltages() {
	for (int i = 0; i < total_channels; i++)
		voltages[i] = -10.0;
}

void loop()
{
	//check for incoming serial stream
	while (Serial.available()) {
		char in = Serial.read();	//read command from incoming stream
		if (in != '\n') {		//if the character isn't the end of line marker
							
			command += in;
		}
		else if (in == '\n' && command != "") {	//else if the character is the end of line marker and the command isn't empty, look for a match in command list
											
			Serial.print("\n>"); Serial.print(command);		//echo commands to the user
			if (command == "SET" || command == "set") {
				Serial.print("\nEnter "); Serial.print(total_channels); Serial.print(" floats, separated by commas.\n");
				for (int i = 0; i < total_channels; i++) {	//for every DAC channel			
					voltages[i] = Serial.parseFloat();		//parse a float from the serial stream
				}
				DACset();	//set the DAC channel voltages
				Serial.print("\nVoltages set at: \n"); 
				printFloats(voltages);
			}
			if (command == "SETSINGLE" || command == "setsingle") {		//set the voltage on a single channel
				Serial.print("\nEnter a channel index from 0 to "); Serial.print(total_channels - 1); Serial.print(".\n");
				int x = Serial.parseInt();		//parse the channel index from the stream
				Serial.print("\nEnter a voltage value.\n");
				voltages[x] = Serial.parseFloat();		//parse the voltage value from the stream

				DACset();	
				Serial.print("\nVoltages set at: \n");
				printFloats(voltages);
			}
			else if (command == "RANDOM" || command == "random") {		//set all channel voltages to random values on the scale
				time_t timer;
				time(&timer);
				randomSeed(timer);		//initialize random seed for voltage generation
				for (int i = 0; i < total_channels; i++) {	//for every DAC channel
					voltages[i] = random(-1000, 1001) / 100.0;	//random float between -10 and 10. 1001 was used because random upper bound is exclusive.
				}
				DACset(); //set the DAC channel voltages
				Serial.print("\nVoltages set at: \n");
				printFloats(voltages);
			}
			else if (command == "DEFAULT" || command == "default") {	//set each channel to a unique, known voltage
				for (int i = 0; i < total_channels; i++) {	//for every DAC channel
					voltages[i] = i * .1;	//channel 1 is .1V, channel 2 is .2V, . . . channel 44 is 4.4V
				}
				DACset();	//set the DAC channel voltages
				Serial.print("\nVoltages set at: \n");
				printFloats(voltages);
			}
			//else if (command == "SETSCALE" || command == "setscale") {	//divide the voltage spectrum equally between the channels
			//	float increment = 20.0 / total_channels;	//voltage increment between channels
			//	float currentVolts = -10.0;			//track the current voltage
			//	for (int i = 0; i <= total_channels; i++) {	//for every DAC channel
			//		voltages[i] = currentVolts;			//assign the voltage
			//		currentVolts += increment;			//increment the voltage
			//	}
			//	DACset();	//set the channels
			//	Serial.print("\nVoltages set at: \n");
			//	printFloats(voltages);
			//}
			else if (command == "CLEAR" || command == "clear") {	//clear all the DAC channels to the lower rail
				DAC.ClearDACs();	//clear the DAC channels
				clearVoltages();	//clear the voltage array to the lower rail
				Serial.print("\nDAC channel voltages cleared to the lower rail.\n");
			}
			else if (command == "boardID" || command == "boardid") {	//print the board ID
				Serial.print("\nThis is Arduino #"); Serial.print(boardID); Serial.print(".\n");
			}
			else if (command == "setBoardID" || command == "setboardid") {	//set the board ID, write to EEPROM
				Serial.print("\nEnter an integer.\n");
				boardID = Serial.parseInt();
				writeID();
				Serial.print("\Board ID set to "); Serial.print(boardID); Serial.print(".\n");
			}
			//else if (command == "SETCOEFFICIENTS" || command == "setcoefficients") {	//set the x- and y-coefficients, save to EEPROM
			//	Serial.print("\nEnter "); Serial.print(total_channels); Serial.print(" floats for the x-coefficients, separated by commas.\n");
			//	for (int i = 0; i < total_channels; i++) {		//for every DAC channel
			//		xCoef[i] = Serial.parseFloat();	//parse a float from the serial stream
			//	}
			//	writeX();		//write the new coefficient values to the EEPROM
			//	Serial.print("\X-coefficients set at: \n");
			//	printFloats(xCoef);

			//	Serial.print("\nEnter "); Serial.print(total_channels); Serial.print(" floats for the y-coefficients, separated by commas.\n");
			//	for (int i = 0; i < total_channels; i++) {		//for every DAC channel
			//		yCoef[i] = Serial.parseFloat();	//parse a float from the serial stream
			//	}
			//	writeX();		//write the new coefficient values to the EEPROM
			//	Serial.print("\Y-coefficients set at: \n");
			//	printFloats(yCoef);
			//}
			//else if (command == "COEFFICIENTS" || command == "coefficients") {	//reload the x- and y-coefficients from EEPROM, display
			//	loadX();
			//	Serial.print("\X-Coefficients loaded:\n");
			//	printFloats(xCoef);		//display the coefficients just loaded in

			//	loadY();
			//	Serial.print("\Y-Coefficients loaded:\n");
			//	printFloats(xCoef);		//display the coefficients just loaded in
			//}
			else if (command == "RTDMUX" || command == "rtdmux") {
				Serial.print("\nEnter 5 bits, LSB first, separated by commas.\n");
				int x[5];		//array for holding bits
				for (int i = 0; i < 5; i++) {	//get 5 bits from the serial stream
					x[i] = Serial.parseInt();
				}

				Serial.print("\nRTD MUX bits set at (LSB) "); Serial.print(x[0]);
				digitalWrite(1, x[0]);			//ARD_S1_A0
				for (int i = 1; i < 5; i++) {	//ARD_S2_A0 through ARD_S2_A3
					Serial.print(", "); Serial.print(x[i]);
					digitalWrite(i + 3, x[i]);
				}
				Serial.print(" (MSB).\n");
			}
			else if (command == "VERSION" || command == "version") {
				versionInfo();
			}
			command = "";
		}
	}
	menu();		//print main menu
	while (!Serial.available()) {	//wait for more serial input
		;
	}
}