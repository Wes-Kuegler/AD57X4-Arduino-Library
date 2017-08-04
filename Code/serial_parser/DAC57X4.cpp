/* Version: 1.0.3
 * Copyright (c) 2013 by Laser-Lance Fordham <Lance.Fordham@gmail.com>
 * DAC57X4.cpp - Library for the Analog Devices Inc. AD5724/AD5734/AD5754 Quad precision [12/14/16-bit] Digital to Analog Converters.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 *
 * modified by Bernd Rilling (brilling@ifsw.uni-stuttgart.de)
 *
 * modified by Wesley Kuegler (wes.kuegler@gmail.com) in Summer 2017 at NASA's Langley Research Center	
 */

#include "DAC57X4.h"

/*6 arguments: number of DAC channels to be used (probably the max: 4), number of DACs being 
chained together (1 will disable daisy-chaining features), DAC output voltage range 
(represented by an int corresponding to a switch case in ConfigDACs), Arduino pin number for SYNC
line, Arduino pin number for CLR line, Arduino pin number for LDAC line*/
DAC57X4::DAC57X4(int numChans1, int numDACs1, int volt, int sync, int clr, int ldac){
	sync_pin = sync;				//set SYNC line pin
	clr_pin = clr;				//Arduino pin mapped to CLR line
	ldac_pin = ldac;
	
	pinMode(sync_pin, OUTPUT);		//Set pin as output
	digitalWrite(sync_pin, HIGH);	//Put selected SS high

	pinMode(clr_pin, OUTPUT);		//initialize the CLR signal line (it's normally HIGH)
	digitalWrite(clr_pin, LOW);		//cycle CLR
	digitalWrite(clr_pin, HIGH);

	pinMode(ldac_pin, OUTPUT);		//initialize the LDAC signal line (It's normally HIGH)
	digitalWrite(ldac_pin, HIGH);
	
	SPI.begin();					//initialize SPI stuff (used in SendData())
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE2);
	SPI.setClockDivider(SPI_CLOCK_DIV2);

	voltSwitch = volt;	//integer corresponding to switch case (currently commented out) in ConfigDACs
									//and in pushDACvoltage(). Sets the voltage range
	numChans = numChans1;		//number of DAC channels being used. Probably all of them (4)
	numDACs = numDACs1;			//the number of DACs being daisy-chained. 1 disables chaining

	DAC57X4::ConfigDACs();		//configure the DACs for use
	DAC57X4::PowerDACs();
}

//Configure numDACS number of DACs, then powers them on
void DAC57X4::ConfigDACs(){
	/*
	0x180000 = NOP [for readback operations]
	--------------------------------------------------------
	OR the following together based on the options desired
	0x190000 = CTRL Address
	0x1 = SDO Disable Address
	0x2 = CLR Select Address
	0x4 = Clamp Enable Address
	0x8 = TSD Enable Address
	---------------------------------------------------------
	0x1C0000 = Clear Data Address
	---------------------------------------------------------
	0x1D0000 = Load Data Address
	---------------------------------------------------------
	*/
	unsigned long config = 0x19000E;	//startup command that each DAC must receive

	for (int i = 0; i < numDACs; i++)	//send the same config data to each DAC
		SendData(config);
	SYNCdata();							//Sync the data in each input shift register
	
	unsigned long output;

	//for daisy-chain operation, the code in this switch block should have no effect on the 
	//program's operation. Despite this, commenting it out causes DAC initialization to fail.
	switch(numChans){
		case 4:
			/*
			---------------------------------------------------------
			Output scale values. Must have overhead supply in order to accomodate
			0xB0000 = 5V
			0xB0001 = 10V
			0xB0002 = 10.8V
			0xB0003 = +-5V
			0xB0004 = +-10V
			0xB0005 = +-10.8V
			---------------------------------------------------------

			*/
			switch(voltSwitch){
				case 1: //0-5V
					output = 0xB0000;
				break;
				case 2: //0-10V
					output = 0xB0001;
				break;
				case 3: //0-10.8V
					output = 0xB0002;
				break;
				case 4: //+-5V
					output = 0xB0003;
				break;
				case 5: // +-10V
					output = 0xB0004;
				break;
				case 6: //+-10.8V
					output = 0xB0005;
				break;
			}

		case 3:
       		// fixed typo by Bernd Rilling 12/2013
			// 0x50000 should be 0xA0000
			/*
			---------------------------------------------------------
			Output scale values. Must have overhead supply in order to accomodate
			0xA0000 = 5V
			0xA0001 = 10V
			0xA0002 = 10.8V
			0xA0003 = +-5V
			0xA0004 = +-10V
			0xA0005 = +-10.8V
			---------------------------------------------------------

			*/
			switch(voltSwitch){
				case 1: //0-5V
					output = 0xA0000;
				break;
				case 2: //0-10V
					output = 0xA0001;
				break;
				case 3: //0-10.8V
					output = 0xA0002;
				break;
				case 4: //+-5V
					output = 0xA0003;
				break;
				case 5: //+-10V
					output = 0xA0004;
				break;
				case 6: //+-10.8V
					output = 0xA0005;
				break;
			}

		case 2:
			/*
			---------------------------------------------------------
			Output scale values. Must have overhead supply in order to accomodate
			0x90000 = 5V
			0x90001 = 10V
			0x90002 = 10.8V
			0x90003 = +-5V
			0x90004 = +-10V
			0x90005 = +-10.8V
			---------------------------------------------------------

			*/
			switch(voltSwitch){
				case 1: //0-5V
					output = 0x90000;
				break;
				case 2: //0-10V
					output = 0x90001;
				break;
				case 3: //0-10.8V
					output = 0x90002;
				break;
				case 4: //+-5V
					output = 0x90003;
				break;
				case 5: //+-10V
					output = 0x90004;
				break;
				case 6: //+-10.8V
					output = 0x90005;
				break;
			}
		case 1:
			/*
			---------------------------------------------------------
			Output scale values. Must have overhead supply in order to accomodate
			0x80000 = 5V
			0x80001 = 10V
			0x80002 = 10.8V
			0x80003 = +-5V
			0x80004 = +-10V
			0x80005 = +-10.8V
			---------------------------------------------------------

			*/
			switch(voltSwitch){
				case 1: //0-5V
					output = 0x80000;
				break;
				case 2: //0-10V
					output = 0x80001;
				break;
				case 3: //0-10.8V
					output = 0x80002;
				break;
				case 4: //+-5V
					output = 0x80003;
				break;
				case 5: //+-10V
					output = 0x80004;
				break;
				case 6: //+-10.8V
					output = 0x80005;
				break;
			}
		break;
	}

	/*SendData(output);		//uncomment here, comment 4 for loops below to return to original single-DAC functionality
	SYNCdata();*/

	/*the following operates separately from the switches above, because it never sends "output" through SendData().
	It is currently hard-coded to support 4 DAC channels. It configures all of the DACs in the chain for the +/- 10V range*/
	for (int i = 0; i < numDACs; i++)	//for every DAC in the chain
		SendData(0x080004);			//configure channel A
	SYNCdata();				//latch all 11 registers

	for (int i = 0; i < numDACs; i++)	//for every DAC in the chain
		SendData(0x090004);			//configure channel B
	SYNCdata();				//latch all 11 registers

	for (int i = 0; i < numDACs; i++)	//for every DAC in the chain
		SendData(0x0A0004);			//configure channel C
	SYNCdata();				//latch all 11 registers

	for (int i = 0; i < numDACs; i++)	//for every DAC in the chain
		SendData(0x0B0004);			//configure channel D
	SYNCdata();				//latch all 11 registers
	}

//Power up numDACs number of DACS
void DAC57X4::PowerDACs(){
	unsigned long output;	//output to send to the DACs

	switch(numChans){
		case 1:
			output = 0x100001; //Send B000100000000000000000001 to initialize only the first DAC
		break;
		case 2:
			output = 0x100003; //Send B000100000000000000000011 to initialize 2 DACs
		break;
		case 3:
			output = 0x100007; //Send B000100000000000000000111 to initialize 3 DACs
		break;
		case 4:
			output = 0x10000F; //Send B000100000000000000001111 to initialize 4 DACs
		break;
	}
	for (int i = 0; i < numDACs; i++)	//send the same power command to each DAC
		SendData(output);

	SYNCdata();							//Sync the data in each input shift register
}

/*Pushes 24 bits to the first DAC in the daisy chain.
Subsequent pushDACvoltage() calls push voltage data out to further DACs in sequence. When
daisy chaining, the first voltage pushed out will go to the last DAC in the chain.*/
void DAC57X4::pushDACvoltage(float DACVoltage, int DACNumber){ 
	unsigned long DACAddress;
	unsigned long data; //data is the voltage requested from the user application
	unsigned int DACValue;

	/*
	-------------------------------------------------------------------
	The DACValue needs to be calculated from the voltage input by the
	user i.e.

	+4x2.5x(1/65536) = B0000 0000 0000 0000 0000 0001 = 0.000152587890625V
	+4x2.5x(65535/65536) = 9.999847412109375V

	Therefore:

	DACValue = (voltage / 10) * 65536
	-------------------------------------------------------------------
	*/

	switch(voltSwitch){	//set in class constructor; determines voltage range
		case 1: //0-5V
			DACVoltage = constrain(DACVoltage,0.0,4.999923706054688);
			DACValue = (DACVoltage/5) * 65536;
		break;
		case 2: //0-10V
			DACVoltage = constrain(DACVoltage,0.0,9.999847412109375);
			DACValue = (DACVoltage/10) * 65536;
		break;
		case 3: //0-10.8V
			DACVoltage = constrain(DACVoltage,0.0,9.999847412109375);
			DACValue = (DACVoltage/10.8) * 65536;
		break;
		case 4: //+-5V
			DACVoltage = constrain(DACVoltage,-4.999847412109375,4.999847412109375);
			DACValue = (DACVoltage/5) * 32768;			
		break;
		case 5: //+-10V
			DACVoltage = constrain(DACVoltage,-9.99969482421875,9.99969482421875);
			DACValue = (DACVoltage/10) * 32768;
		break;
		case 6: //+-10.8
			DACVoltage = constrain(DACVoltage,-10.79967041015625,10.79967041015625);
			DACValue = (DACVoltage/10.8) * 32768;
		break;
	}

	switch(DACNumber){	//determines which DAC channel to set. 
						//User-inputted as an argument to pushDACvoltage
		case 1:
			DACAddress = 0x00000;
			data = DACAddress | DACValue;
		break;
		case 2:
			DACAddress = 0x10000;
			data = DACAddress | DACValue;
		break;
		case 3:
			DACAddress = 0x20000;
			data = DACAddress | DACValue;
		break;
		case 4:
			DACAddress = 0x30000;
			data = DACAddress | DACValue;
		break;
		case 5:		//all 4 DAC channels
			DACAddress = 0x40000;
			data = DACAddress | DACValue;
		break;
	}

	data = data ^ 0x008000;	/*MASK. Invert the MSB of the data to switch 
							from twos complement to offset binary*/

	SendData(data);	/*push the voltage data out to the chain
	no call is made to SYNCdata because pushDACvoltage will be called externally, several
	times in succession. A SYNCdata call must be made externally once all pushing is complete*/
}

void DAC57X4::LoadDACs(){		//send a load command to every DAC in the chain

	unsigned long data = 0x1D0000;

	for (int i = 0; i < numDACs; i++)	//send the same load command to every DAC
		DAC57X4::SendData(data);
	
	digitalWrite(ldac_pin, LOW);				//set LDAC LOW
	SYNCdata();									//Sync the data in each DAC's input shift register
	digitalWrite(ldac_pin, HIGH);				//set LDAC HIGH
}

void DAC57X4::ClearDACs(){		//send the clear command to every DAC in the chain

	unsigned long data = 0x1C0000;

	for (int i = 0; i < numDACs; i++)	//send the same clear signal to every DAC
		DAC57X4::SendData(data);
	SYNCdata();							//Sync the data in each input shift register
}

//NOT WORKING. Uses ReadDACsRegister to read the DAC registers of all 4 channels on each chained DAC
//void DAC57X4::ReadDACs(byte** raw){
//	//for each of the 4 channels, read out from all 11 DACs
//	for (int i = numChans; i > 0; i--) {
//		ReadDACsRegister(1, i, raw);	//ReadDACsRegister will do all 11 at once, 
//										//putting the register dumps in the correct indices
//	}
//}
//
//void DAC57X4::ReadDACsRegister(int DACRegister, int DACNumber, byte** raw){
//
//	unsigned long DACAddress;
//	unsigned long data;
//	unsigned long DACValue;
//
//	/*
//	-------------------------------------------------------------------
//	DACRegister
//	1 = DAC register
//	2 = Output range select register
//	3 = Power control register
//	4 = Control register
//	-------------------------------------------------------------------
//	*/
//
//	switch(DACNumber){
//		case 1: //DAC A
//			DACAddress = 0x00000;
//		break;
//		case 2: //DAC B
//			DACAddress = 0x10000;
//		break;
//		case 3: //DAC C
//			DACAddress = 0x20000;
//		break;
//		case 4: //DAC D
//			DACAddress = 0x30000;
//		break;
//	}
//
//	switch(DACRegister){
//		case 1:	//DAC register
//			DACValue = 0x800000;
//			data = DACValue | DACAddress;
//		break;
//		case 2:	//Output range select register
//			DACValue = 0x880000;
//			data = DACValue | DACAddress;
//		break;
//		case 3:	//Power control register
//				//DACNumber don't care
//			data = 0x900000;
//		break;
//		case 4:	//Control register
//				//DACNumber don't care
//			data = 0x990000;
//		break;
//	}
//	//Send read command for the selected register and DACNumber
//	for (int i = 0; i < numDACs; i++) {		//send the same command to every DAC in the chain
//		SendData(data);
//	}
//
//	SYNCdata();		//latch the read operation
//	//digitalWrite(sync_pin, LOW);	//return SYNC line to low (this happens in SendData() anyway so shouldn't be necessary
//
//	for (int i = 0; i < numDACs; i++) {	//for every DAC
//		SendData(0x180000);	//Send B0001 8000 0000 0000 0000 0000 = NOP (no operation) for readbacks
//	}
//
//	SYNCdata();		//latch the NOP. Selected register will clock out on SDO of each DAC
//
//	for (int i = numDACs; i > 0; i--){	//for every DAC (skipping index 0. Simplifies firmware side)
//		for (int j = 2; j >= 0; j--){	//FUTURE TROUBLESHOOTING: consider making this happen only once per DAC
//				raw[(i*4) - (4-DACNumber)][j] = SPI.transfer(0x180000);	//send another NOP condition. This one is just
//									//for pushing data out SDO lines and further down the chain		
//		}
//	}
//	SYNCdata();
//}

/*sends data from a long out along the chain. holds SYNC low for the sending operation.
MUST be followed by a call to SYNCdata when done writing data. SYNCdata exists as a separate 
function so that SendData can be called repeatedly (to push data along the daisy chain)*/
void DAC57X4::SendData(long data){ 

	digitalWrite(sync_pin,LOW);

	unsigned char *p = (unsigned char*)&data;	/*cast long data to an unsigned char array.
												each unsigned char is 8 bits.*/
	for (int i=2; i>= 0; i--){ /*Send 3 chars, totaling the 24 bits needed to fill 
							   the DAC's input shift register. MSB first*/
		SPI.transfer(p[i]);
	}
}

/*MUST be called when done pushing out data with SendData. Sends a high signal on the SYNC line.
In single DAC mode, simply make a SYNCdata() call after every SendData() call. In daisy-chain mode,
use SendData() to load every input shift register in the chain; once they are all loaded, call SYNCdata()
to latch the commands*/
void DAC57X4::SYNCdata() {
	digitalWrite(sync_pin, HIGH);
}