//**************************************************************************
//
// This file contains any functions that do not fall under any specific
// category.
//
//***************************************************************************

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
#include "modbus.h"
#include "misc.h"

static unsigned int timerstate;

//*********************************************************************************************
//
// This function provides the interrupt handler for the delay function.
//
//*********************************************************************************************

void Timer0IntHandler(void)
{
	timerstate = 1; // Sets flag showing that the interrupt has triggered
	TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Clear the timer interrupt
}

//*********************************************************************************************
//
// This function outputs a character on the USB.
//
//*********************************************************************************************

void myCharPut(char c) // Wrapper function for UART Character Put Function
{
	UARTCharPut(UART1_BASE,c);
}

//*********************************************************************************************
//
// This function outputs a string, character by character, to the USB.
//
//*********************************************************************************************

void myStringPut(char * cptr)  // Outputs a NULL terminated string
{
	while ( ( * cptr ) != '\0' ) // While current character is not NULL terminator
	{
		myCharPut(* cptr); // Output current character
		cptr++; // Advance pointer
	}
}

//*********************************************************************************************
//
// This function outputs pulls a character from over the blue tooth.
//
//*********************************************************************************************

char myCharGet(void) // Wrapper function for UART Character Get Function
{
	return UARTCharGet(UART1_BASE);
}

//*********************************************************************************************
//
// This function determines rather a character is available over the blue tooth.
//
//*********************************************************************************************

bool myCharsAvail(void) // Wrapper function for UART Characters Available in buffer
{
	return UARTCharsAvail(UART1_BASE);
}

//*********************************************************************************************
//
// This function will convert an ASCII Hex Digit to an integer value.
//
//*********************************************************************************************

unsigned int char2Int( char ch)
{
	if ( ('0' <= ch) && (ch <='9') )
	{
		return (ch -'0');
	}
	else if ( ('A' <= ch) && (ch <= 'F'))
	{
		return (ch - 'A' + 10);
	}
	return 999; // Return '999' in the event of an error.
}

//*********************************************************************************************
//
// This function will enable the UART controlling the blue tooth.
//
//*********************************************************************************************

void myBT_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1); // Enable UART1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // Enable Port B
	GPIOPinConfigure(GPIO_PB0_U1RX); // Configure GPIO PB0 for UART Receive
	GPIOPinConfigure(GPIO_PB1_U1TX); // Configure GPIO PB1 for UART Transmit
	GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1); // set PB0 & PB1
	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)); // Configure UART1 clock, etc.
}

//*********************************************************************************************
//
// This function outputs new line characters.
//
//*********************************************************************************************

void myNLPut(void)
{
	myCharPut(0x0D);
	myCharPut(0x0A);
}

//*********************************************************************************************
//
// This function initializes the onboard LED.
//
//*********************************************************************************************

void myLEDInit(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Enable Port F
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); // Sets LED output pins
}

//*********************************************************************************************
//
// This function provides an interrupt based delay given in microseconds.
//
//*********************************************************************************************

void delay(unsigned int countus)
{
	unsigned int delay;

	timerstate = 0;
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); //Enable Timer
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	delay = (SysCtlClockGet()/1000000)*countus;
	TimerLoadSet(TIMER0_BASE, TIMER_A, delay -1);
	IntEnable(INT_TIMER0A);	//Enable interrupts
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A);
	while(timerstate == 0) //Spin until timerstate is 1
	{
	}
}

//*********************************************************************************************
//
// This function will output a string to across the USB.
//
//*********************************************************************************************

void usbStringPut(char * cptr)  // Outputs a NULL terminated string
{
	while ( ( * cptr ) != '\0' ) // while current character is not NULL terminator
	{
		UARTCharPut(UART0_BASE,* cptr); // output current character
		cptr++; // advance pointer
	}
}
void uint2str(unsigned int val, char * sout)
{
	ltoa( (long int) val,sout);
}
