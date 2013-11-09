//**************************************************************************
//
//This file contains all of the functions necessary for operation of the
//command line interpreter.
//
//***************************************************************************

#define PART_TM4C123GH6PM 1 // This defines the current processor used. Necessary to use pin out macros.

#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdlib.h>
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
#include "modbus.h"
#include "sensors.h"
#include "misc.h"
#include "commandinterpreter.h"


#define BLACK 0
#define WHITE 1

typedef struct
{
	char cmdName[256];
	void (*cmdPtr)();

} cmdDef;

#define cmdNumber 17
cmdDef cmdSet[cmdNumber];

char cmdStr[256];
char cmdData[256];
char outstr[256];
int cmdDataLength;
char inChar;
int j;
char flagTimer;

//*********************************************************************************************
//
// This function initializes the command list for the interpreter.
//
//*********************************************************************************************

void myCmdLineInit(void)
{

	strncpy(cmdSet[0].cmdName,"hello",10);
	cmdSet[0].cmdPtr=&cmdHello;
	strncpy(cmdSet[1].cmdName,"clock",10);
	cmdSet[1].cmdPtr=&cmdClock;
	strncpy(cmdSet[2].cmdName,"blink",10);
	cmdSet[2].cmdPtr=&cmdBlink;
	strncpy(cmdSet[3].cmdName,"stop",10);
	cmdSet[3].cmdPtr=&cmdStopBlink;
	strncpy(cmdSet[4].cmdName,"turnon",10);
	cmdSet[4].cmdPtr=&cmdTurnOn;
	strncpy(cmdSet[5].cmdName,"turnoff",10);
	cmdSet[5].cmdPtr=&cmdTurnOff;
	strncpy(cmdSet[6].cmdName,"forward",10);
	cmdSet[6].cmdPtr=&cmdforward;
	strncpy(cmdSet[7].cmdName,"slow",10);
	cmdSet[7].cmdPtr=&cmdslow;
	strncpy(cmdSet[8].cmdName,"stopm",10);
	cmdSet[8].cmdPtr=&cmdstop;
	strncpy(cmdSet[9].cmdName,"distance",10);
	cmdSet[9].cmdPtr=&cmddistance;
	strncpy(cmdSet[10].cmdName,"line",10);
	cmdSet[10].cmdPtr=&cmdline;
	strncpy(cmdSet[11].cmdName,"help",10);
	cmdSet[11].cmdPtr=&cmdHelp;
	strncpy(cmdSet[12].cmdName,"bangbang",10);
	cmdSet[12].cmdPtr=&cmdBangBang;
	//cmdSonicRead
	strncpy(cmdSet[13].cmdName,"sonicread",10);
	cmdSet[13].cmdPtr=&cmdSonicRead;
	strncpy(cmdSet[14].cmdName,"sonicpoll",10);
	cmdSet[14].cmdPtr=&cmdSonicPoll;
	strncpy(cmdSet[15].cmdName,"pid",10);
	cmdSet[15].cmdPtr=&cmdStartPID;
	strncpy(cmdSet[16].cmdName,"starts",10);
	cmdSet[16].cmdPtr=&cmdSonicSample;
}

void cmdSonicSample( void )
{
	sonic_start_thread();

	while(1)
	{
		ltoa( (long int) get_sonic_dist(),outstr);
		myStringPut("dist=");
		myStringPut(outstr);
		myNLPut();
		//SysCtlDelay(SysCtlClockGet()/3);
	}
}

void cmdStartPID(void)
{
	myStringPut("cmdStartPID() called");
	myNLPut();
	sonic_pid();
}

void cmdSonicPoll(void)
{
	unsigned int val;
	while(1)
	{
		cmdSonicRead();
		delay(500000);
	}

}
void cmdSonicRead(void)
{
	char out[128];
	unsigned int sonic_val;
	//myStringPut("cmdSonicRead() called");

	sonic_val=sonic_read();

	myStringPut("sonic_val=");
	ltoa( (long) sonic_val,out);
	myStringPut(out);
	myStringPut(", ");
	ltoa( (long) (sonic_val/50000),out);
	myStringPut(out);
	myStringPut(" [ms]");
	myNLPut();
}


void cmdBangBang(void)
{
	motor_init();
	motor_left_stop();
	motor_right_stop();

	motor_left_duty(20);
	motor_right_duty(40);
	motor_left_forward();
	motor_right_forward();
	while(1)
	{
		// start turning left
		motor_left_duty(20);
		motor_right_duty(40);
		myStringPut("Bang Left");
		myNLPut();
		while (line_status() == WHITE) // WHITE
		{
		}
		// Hits Black

		// Turns right
		motor_left_duty(40);
		motor_right_duty(20);

		myStringPut("Bang Right");
		myNLPut();
		while (line_status() == BLACK) // BLACK
		{
		}
	}
}

//**************************************************************************
//
//This function runs the command line interpreter on infinite loop.
//
//***************************************************************************

void myCmdLine(void)
{
	int k;
	char flagDone,promptDone;

	k=0;
	flagDone=0;
	promptDone=0;
	while (1)
	{
		if (flagDone == 1)
		{
			myCharPut( '\n');						// Debugging output
			myStringPut("Command Recieved:");
			myCharPut('\'');
			myStringPut( cmdStr);
			myCharPut('\'');
			myCharPut(0x0D);
			myCharPut(0x0A);
			myCharPut(checkSumDigit1(checkSum( cmdStr)));
			myCharPut(checkSumDigit0(checkSum( cmdStr)));
			myNLPut();


			if ( myModbusRead(cmdStr,cmdData) ) // extracts DATAFIELD from cmdStr(modbus frame) and writes to cmdData(command name)
			{
				for(j=0;j<(cmdNumber);j++) // iterates through each element of command array
				{
					if ( strcmp(cmdData,cmdSet[j].cmdName) == 0 ) // test if current element of command array is entered command
					{
						( *(cmdSet[j].cmdPtr) )();  // calls function via function pointer
						break;
					}
				}
			}
			flagDone=0; // resets flag done
			promptDone=0; // resets prompt flag
			k=0;  // resets string iterator


		}
		else
		{
			if (promptDone == 0) // if prompt hasn't yet been displayed
			{
				myStringPut("Enter a Command: "); // output prompt
				promptDone = 1; // set prompt flag to 1
			}

		}

		if ( myCharsAvail() )  // if there are characters in buffer
		{
			myCharPut( inChar=myCharGet()); // copy character from UART buffer; echo character
			if ( (inChar == 0x0D) || (inChar == 0x0A) )  // if New Line character encountered
			{
				flagDone = 1; // set Done flag; scan available commands next iteration
				cmdStr[k]='\0'; // add a NULL terminator to string buffer
				k=0;  // restart character buffer counter
			}
			else
			{
				cmdStr[k++]=inChar; // add a character to buffer and increment character index
			}
		}


	}

}

//*********************************************************************************************
//
// This function simple command acts as a "hello world" test.
//
//*********************************************************************************************

void cmdHello(void)
{
	myStringPut("He Mensche!"); // output example string to
	myNLPut();
}

//*********************************************************************************************
//
//
//*********************************************************************************************

void cmdClock(void)
{
	char out0[100];
	unsigned int crate;
	myStringPut("cmd clock");
	myNLPut();
	crate = SysCtlClockGet(); // Calls library function that returns clock rate
	sprintf(out0,"%u Hz",crate);
	myStringPut(out0);
	myNLPut();
}

void cmdBlink(void)
{
	uint32_t ui32Period; // number of ticks per timing period

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  // Configure Timer0
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); // Set Timer0 to periodic operation
	ui32Period = (SysCtlClockGet() * 2) / 2; // Defines period to 2 seconds
	TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1); // Sets period of Timer0 to ui32Period (ticks)
	IntEnable(INT_TIMER0A); // Enable interrupts on Timer0
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Configure interrupt on Timer0
	IntMasterEnable(); // Allow interrupts
	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer0
}

void cmdStopBlink(void)
{
	IntDisable(INT_TIMER0A); // Disable Timer0 interrupt
	cmdTurnOff(); // Turn off LED
}

void cmdTurnOn(void)
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);  // Turn on Blue LED
}

void cmdTurnOff(void)
{
	GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00); // Turn off Blue LED
}

void cmdHelp(void) // Lists Available Commands
{
	char outstr[10];
	int k;
	for(k=0;k<cmdNumber;k++) // iterate through all
	{
		myStringPut( cmdSet[k].cmdName ); // output command name
		outstr[0]=checkSumDigit1(checkSum(cmdSet[k].cmdName));
		outstr[1]=checkSumDigit0(checkSum(cmdSet[k].cmdName));
		outstr[2]='\0';
		myStringPut(outstr);
		myNLPut();
	}
}

void cmdforward(void) // Moves motors forward at full duty
{
	myStringPut("calling cmdforward");
	myNLPut();
	motor_left_duty(100);
	motor_right_duty(100);
	motor_left_forward();
	motor_right_forward();
}

void cmdstop(void) // Stops motors
{
	myStringPut("calling cmdstop");
	myNLPut();
	motor_left_stop();
	motor_right_stop();
}

void cmdslow(void) // Moves motors forward at half duty
{
	motor_left_duty(15);
	motor_right_duty(15);
	motor_left_forward();
	motor_right_forward();
}

void cmddistance(void) // Reads the value of the IR sensor
{
	char strout[100];
	myStringPut("calling cmddistance");
	myNLPut();
	unsigned int distance;
	distance = distance_read();
	sprintf(strout,"dist=%u",distance);
	myStringPut(strout);
	myNLPut();
}

void cmdline(void)
{
	char strout[100];
	myStringPut("calling cmdline");
	myNLPut();
	unsigned int line;
	line = line_read();
	sprintf(strout,"line=%u",line);
	myStringPut(strout);
	myNLPut();

	if (line <= 10)
	{
		sprintf(strout,"WHITE detected");
		myStringPut(strout);
		myNLPut();
	}
	else
	{
		sprintf(strout,"BLACK detected");
		myStringPut(strout);
		myNLPut();
	}
}


