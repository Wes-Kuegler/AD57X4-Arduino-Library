/* Version: 1.0.3
 * Copyright (c) 2013 by Laser-Lance Fordham <Lance.Fordham@gmail.com>
 * DAC57X4.h - Library for the Analog Devices Inc. AD5724/AD5734/AD5754 Quad precision [12/14/16-bit] Digital to Analog Converters.
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

#ifndef DAC57X4_h
#define DAC57X4_h

#include "Arduino.h"
#include <SPI.h>
#include "pins_arduino.h"

class DAC57X4{

		public:

			DAC57X4(int numChans, int numDACs, int voltSwitch, int ss_pin, int clr_pin, int ldac_pin);
			void pushDACvoltage(float DACVoltage, int DACNumber);	//pushes data out to first DAC in chain
			void LoadDACs();					//sends a load register command to every DAC in the chain
			void ClearDACs();					//send a clear command to each DAC in the chain
			void ConfigDACs();					//initializes the DAC or DAC chain
			void PowerDACs();					//another initialization function
			//void ReadDACs(byte** raw);			//NOT WORKING
			//void ReadDACsRegister(int DACRegister, int DACNumber, byte** raw);		//NOT WORKING
			void SendData(long data);			//sends data to first DAC in the chain
			void SYNCdata();					/*sends a SYNC high signal to latch commands after pushing
												data out along the daisy chain*/

			int sync_pin;	//Arduino pin mapped to SYNC line
			int clr_pin;	//Arduino pin mapped to CLR line
			int ldac_pin;	//Arduino pin mapped to LDAC line
			int voltSwitch;	//voltage range. Int used to select a switch case
			int numChans;	//number of DAC channels being used. Probably all of them (4)
			int numDACs;	//the number of DACs being daisy-chained. 1 disables chaining
};

#endif
