// Jerrod McClure
// Caleb Cain
// ECE 4437
// Group #05

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
#include "sensors.h"



void myusbInit(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0); // Enable UART0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // Enable Port A
	GPIOPinConfigure(GPIO_PA0_U0RX); // Configure GPIO PA0 for UART Receive
	GPIOPinConfigure(GPIO_PA1_U0TX); // Configure GPIO PA1 for UART Transmit
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1); // set PB0 & PB1
	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 9600,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE)); // Configure UART1 clock, etc.

}




//*********************************************************************************************
//
// This function will output a string to across the USB.
//
//*********************************************************************************************

//void usbStringPut(char * cptr)  // Outputs a NULL terminated string
//{
//	while ( ( * cptr ) != '\0' ) // while current character is not NULL terminator
//	{
//		UARTCharPut(UART0_BASE,* cptr); // output current character
//		cptr++; // advance pointer
//	}
//}


void usbNLPut(void)
{
	UARTCharPut(UART0_BASE,0x0D);
	UARTCharPut(UART0_BASE,0x0A);
}


void ISR(void)
{
	color_turn_off_led();
	UARTCharPut(UART0_BASE,'I');
	UARTCharPut(UART0_BASE,'S');
	UARTCharPut(UART0_BASE,'R');
	UARTCharPut(UART0_BASE,'(');
	UARTCharPut(UART0_BASE,')');
	usbStringPut("ISR() called");
	UARTCharPut(UART0_BASE,0x0D);
	UARTCharPut(UART0_BASE,0x0A);
}

void ritual()
{
	myusbInit();

	myBT_Init();
	line_init();
	distance_init();
	sonic_init();
	motor_init();
	motor_left_duty(10);
	motor_right_duty(10);
	color_init();
	myCmdLineInit();

}
int main(void)
   {
	char sout[50];
	SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); // Set System Clock
//	myCmdLineInit();
//	myBT_Init();
//	line_init();
//	distance_init();
//	motor_init();
//	myCmdLine();
//
//
//	myusbInit();
//	sonic_init();
//	myBT_Init();
//
//	myCmdLineInit();
//	line_init();
	unsigned int kr,kb;
	ritual();

	ltoa( (long int) SysCtlClockGet(),sout);
	usbStringPut(sout);
	usbNLPut();


}
