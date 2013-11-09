//*********************************************************************************************
//
// This file contains all of the functions necessary to use the MODBUS protocol.
//
// We implement an ASCII Modbus frame of length N
// modbus_frame[0] = ':"
// modbus_frame[1:2] -> Station Address
// modbus_frame[3:4] -> Function
// modbus_frame[5:N-7] -> Data Field
// modbus_frame[(N-6):(N-5)] -> Field Length
// modbus_frame[(N-4):(N-3)] -> LRC check sum
// modbus_frame[(N-2):(N-1)] -> CR + LF

//
//*********************************************************************************************


#define PART_TM4C123GH6PM 1 // This defines the current processor used. Necessary to use pin out macros.

#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include <string.h>
#include "motors.h"
#include "commandinterpreter.h"
#include "misc.h"

char myStationAddress[] = "05"; // The address used our robot will use across the MODBUS.

//*********************************************************************************************
//
// This function will calculate the checksum using the longitudal redundancy check method.
//
//*********************************************************************************************

unsigned int checkSum(char * cptr)
{
	unsigned int acc;
	unsigned char * ptr;

	ptr = (unsigned char *) cptr;
	acc = 0;

	while ( (*  ptr) != '\0') // iterate through NULL terminated character string
	{
		acc += (*ptr);  // accumulate ASCII value of current character
		ptr++;
	}
	acc = acc ^ 0xFF + 1;  // twos complement of Least Significant Byte
	//return (acc % (256));
	return (acc & 0xFF); // return only the Least Significant Byte
}

//*********************************************************************************************
//
// This function returns an ASCII Character of the least significant byte for the checksum.
//
//*********************************************************************************************

char checkSumDigit0( unsigned int cs)
{
	int x;
	x = (cs % 16);
	if (x < 10)
	{
		return x+'0';
	}
	else
	{
		return (x-10)+'A';
	}
}

//*********************************************************************************************
//
// This function returns an ASCII Character of the second least significant byte for the checksum.
//
//*********************************************************************************************

char checkSumDigit1( unsigned int cs)
{
	return checkSumDigit0( cs/16);
}

char myModbusRead(char * modbus, char * cmdName0)
{
	if (strlen(modbus) > 8) // checks if Modbus frame is at least as long as the minimum length
	{
		if (modbus[0] == ':') // checks if Modbus frame has start character
		{
			if ( (modbus[1] == myStationAddress[0]) && (modbus[2] == myStationAddress[1])  )  // checks if Station Address is Correct
			{
				int cdl;
				cdl = 16*char2Int( modbus[strlen(modbus)-4])+char2Int( modbus[strlen(modbus)-3]); // strip length of DATAFIELD from MODBUS frame
//				myStringPut(outstr);  ***DELETE THIS LINE
				myCharPut(0x0D);
				myCharPut(0x0A);

				if ( (strlen(modbus)-9) == cdl) // check if DATAFIELD length is correct
				{
					int k1;
					myStringPut("Modbus is formated correctly");
					myNLPut();

					for(k1=0;k1<cdl;k1++) // iterate through each character of the DATAFIELD
					{
						cmdName0[k1]=modbus[k1+5]; // copy DATAFIELD to string
					}
					cmdName0[k1]='\0'; // add NULL terminator at the end
					myNLPut();
					myStringPut("Data Field:"); // Echo command out
					myStringPut(cmdName0);
					myNLPut();

					// verify checksum bytes
					if( (checkSumDigit0(checkSum( cmdName0)) == modbus[strlen(modbus)-1]) && (checkSumDigit1(checkSum( cmdName0)) == modbus[strlen(modbus)-2]) )
					{
						return 1; // return 1 on properly formated Modbus frame
					}
				}

			}
		}
	}
	return 0;  // return 0 on any violations of Modbus frame
}

