"""
#Author: Wesley Kuegler, Summer 2017 NIFS intern at NASA's Langley Research Center
#Date: 7/26/17
This file contains Python functions for operating the AD57X4 Arduino Micro firmware (written in #C++) found in 
serial_parser.ino. The best way to use these functions is to simply import them into another Python file. 

==============================================IN THIS VERSION, V17.7.26=============================================
*Added getPort() function, which returns the label of the port with a connected Arduino. Replaced the hard-coded
	port name in the port opening function with a call to getPort()
*removed getResistances() because the resistances are no longer stored on the Arduino side
*added getID() and setID() functions for getting and setting the ID of the connected Arduino

=============================================OLDER VERSION INFORMATION=============================================
======================================================V17.7.7======================================================
*Added confirmCommand() function, which checks for feedback from the Arduino after sending commands. If no feedback
    is read, the program exits with an error code.
*Added confirmCommand() calls at the end of every Arduino-operating function

======================================================V17.7.7======================================================
*All of the wrapper functions are working as intended

"""

import serial
import time
import sys
import serial.tools.list_ports as SerialPort
import re

#takes a list of voltages as an argument and pushes them out to the DACs. Two of the DAC channels are not connected
#to anything, but the list should still contain 44 numbers if using the MEDLI2 SSE Sensor Simulator board
def setVoltages(voltages):  
    port.write("set\n".encode());
    first = True
    for x in voltages:
        if first:
            first = False
        else:
            port.write(','.encode())
        port.write(str(x).encode())
        
    port.write("\n".encode())
    confirmCommand()
    
#sets the voltages to .1 of their channel index, i.e. Channel 1 = .1V, 
def setDefault():
    port.write("default\n".encode())
    confirmCommand()
    
#divides the voltage range equally amongst the channels, with an increment of (voltage range) / (number of channels) between each
#the setScale command in the firmware has been deprecated
#def setScale():
#    port.write("setscale\n".encode())
#    confirmCommand()
    
#sets all channels to random values on the voltage scale
def setRandom():
    port.write("random\n".encode())
    confirmCommand()
    
#clears all channel values
def clear():
    port.write("clear\n".encode())
    confirmCommand()

#display version info        
def version():
    port.write("version\n".encode())
    confirmCommand()

#set the 5 RTD MUX bits, LSB first        
def setRTD(bits):
    port.write("RTDMUX\n".encode())
    first = True
    for x in bits:
        if first:
            first = False
        else:
            port.write(','.encode())
        port.write(str(x).encode())
        
    port.write("\n".encode());
    confirmCommand()
        
#wait .001 seconds for some kind of response from the Arduino
def confirmCommand():
    time.sleep(.001)     #this time could probably be smaller
    if not port.inWaiting():
        sys.exit("Arduino took too long to respond!")
		
#return the port label for the connected Arduino		
def getPort():
	ports = list(SerialPort.comports())

	for this_port in ports:
		port_string = str(this_port)

		if "Arduino" in port_string:
			
			return ( "COM" + str( re.search(r'\d+', port_string).group() ) )
	
	print("Arduino not found!")
	return None

#return the hardware ID of the Arduino 
def getID():
    port.write("boardid\n".encode())
    confirmCommand()
    
#set the hardware ID of the Arduino
def setID(ID):
    port.write("setboardid\n".encode())
    port.write((str(ID) + "\n").encode())
    confirmCommand()

port = serial.Serial(getPort(), 9600) #open port. port name will change from pc to pc, so call 

#sample list for setting all channels at increasing voltages on a .5V increment. 10 is repeated once the end of the scale is reached
scaleList = [-10, -9.5, -9, -8.5, -8, -7.5, -7, -6.5, -6, -5.5, -5, -4.5, -4, 3.5, -3, -2.5, -2, -1.5, -1, -0.5, 0, .5, 1, 1.5, 
          2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 10, 10, 10]
bitList = [1,1,1,0,0]	#sample list of RTD mux bits

#just some function calls for demonstration purposes
version()
setRTD(bitList)
setVoltages(scaleList)
getResistances() 
getID()
setID(1)
getID()

#print out the serial data from the Arduino
time.sleep(1)
while (port.inWaiting()):
        print (port.readline().decode())
  
#flush and close the serial port
port.flush()
port.flushInput()
port.flushOutput()
port.close()