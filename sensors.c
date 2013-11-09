//*********************************************************************************************
//
// This file contains the drivers for the robot sensors.
//
//*********************************************************************************************


#define PART_TM4C123GH6PM 1 // This defines the current processor used. Necessary to use pin out macros.

#define SONIC_PID_SETPOINT 29000

#include "driverlib/pin_map.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
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
#include "driverlib/adc.h"

static unsigned int timer1_start;
static unsigned int timer1_stop;
static unsigned int portd_edge_status;
static char timer1_action;
static char color_flag;
static unsigned int color_counter;
static signed int dist;
static unsigned int sonic_cont;

static char out2[20];
#define COLOR_ACTION 1
#define SONIC_ACTION 0

#define RED 0
#define BLUE 1
#define CLEAR 2
#define GREEN 3
#define BLACK 0
#define WHITE 1
//*********************************************************************************************
//
// This function enables the line sensor port. Port PC4 is used, and will be configured as GPIO.
//
//*********************************************************************************************

void Timer1IntHandler(void)
{
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	if (timer1_action == COLOR_ACTION)
	{
		color_flag = 1;
		// do frequency counting
	}
	else
	{
		portd_edge_status=2;
	}

}


void line_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4);
}

//*********************************************************************************************
//
// This function reads in the current line state.
//
//*********************************************************************************************

unsigned int line_read(void)
{
	unsigned int capstatus;
	unsigned int delaycount;

	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_4); // Set GPIO PC4 to output
	GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4,16); // Write a 1 to the capacitor
	delay(100); //Delay 10 microseconds for the capacitor on the sensor to charge
	GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4); //Switch GPIO to input to measure capacitor
	capstatus = (GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_4) & 0x10) >> 4; //Sets the capacitor charge status, which will be high after a charge, shifts bit 4 to specify 1 or 0.
	delaycount = 0;
	while(capstatus) //Wait for the capacitor to go "low"
	{
		delay(10); //Measuring in a resolution of 10 microseconds
		delaycount++; //Measure the number of times the loop has occurred
		capstatus = (GPIOPinRead(GPIO_PORTC_BASE,GPIO_PIN_4) & 0x10) >> 4;
	}
	return delaycount;
}



extern char line_status(void)
{

	unsigned int line;
	line = line_read();
	if (line <= 10)
	{
		return WHITE;
	}
	else
	{
		return BLACK;
	}
}

//*********************************************************************************************
//
// This function enables the IR distance sensor.
//
//*********************************************************************************************

void distance_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	GPIOPinTypeADC(GPIO_PORTB_BASE,GPIO_PIN_5);
	ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE,1,0,ADC_CTL_CH11|ADC_CTL_IE|ADC_CTL_END);
	ADCSequenceEnable(ADC0_BASE, 1);
}

//*********************************************************************************************
//
// This function takes a measurement from the distance sensor
//
//*********************************************************************************************

unsigned int distance_read(void)
{
	unsigned int *distance;
	unsigned int data;

	distance=&data;

	ADCIntClear(ADC0_BASE, 1); // Clear interrupt flag
	ADCProcessorTrigger(ADC0_BASE, 1); // Trigger ADC read
	while(!ADCIntStatus(ADC0_BASE, 1, false)) // Wait for interrupt
	{
	}
	ADCSequenceDataGet(ADC0_BASE, 1, distance);
	return *distance;
}

//*********************************************************************************************
//
// This function enable the ultrasonic sensor. It uses PD0 for the trigger, and
// PD1 for the echo.
//
//*********************************************************************************************

void sonic_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); //Enable Timer
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3); //Enable Timer

	SysCtlPeripheralEnable(GPIO_PORTA_BASE); //Enable GPIO

	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);

	IntEnable(INT_GPIOA);

}


//*********************************************************************************************
//
// This function will start measuring the pulse width of the sonic sensor output.
//
//*********************************************************************************************

void PortAIntHandler(void)
{

	GPIOIntClear(GPIO_PORTA_BASE,GPIO_INT_PIN_3);
	//GPIOIntDisable(GPIO_PORTA_BASE, GPIO_PIN_3);
	//  	char outstr[100];
	//	sprintf(out,"PortAIntHandler called, portd_edge_status=%u,timervalue=%u",portd_edge_status,TimerValueGet(TIMER1_BASE,TIMER_A));
	//	usbStringPut(out);
	//	UARTCharPut(UART0_BASE,0x0D);
	//	UARTCharPut(UART0_BASE,0x0A);

	// Debug

	if (sonic_cont == 0)
	{
		if (portd_edge_status == 0)
		{

			GPIOIntDisable(GPIO_PORTA_BASE, GPIO_PIN_3);
			GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE);
			portd_edge_status = 1;
			timer1_start=TimerValueGet(TIMER1_BASE,TIMER_A);
			//		ltoa( (long int) timer1_start,outstr);
			//		usbStringPut("timer1_start=");
			//		usbStringPut(outstr);

			GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_3);
		}
		else
		{
			timer1_stop=TimerValueGet(TIMER1_BASE,TIMER_A);
			//		ltoa( (long int) timer1_stop,outstr);
			//		usbStringPut("timer1_stop=");
			//		usbStringPut(outstr);
			portd_edge_status=2;
		}
	}
	else if (sonic_cont == 1)
	{
		if (portd_edge_status == 0)
		{

			GPIOIntDisable(GPIO_PORTA_BASE, GPIO_PIN_3);
			GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE);
			portd_edge_status = 1;
			timer1_start=TimerValueGet(TIMER1_BASE,TIMER_A);
			//		ltoa( (long int) timer1_start,outstr);
			//		usbStringPut("timer1_start=");
			//		usbStringPut(outstr);

			GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_3);
		}
		else if (portd_edge_status == 1)
		{
			timer1_stop=TimerValueGet(TIMER1_BASE,TIMER_A);
			//		ltoa( (long int) timer1_stop,outstr);
			//		usbStringPut("timer1_stop=");
			//		usbStringPut(outstr);
			dist = timer1_start - timer1_stop;

			// Setup timer interrupt


			TimerConfigure(TIMER3_BASE, TIMER_CFG_PERIODIC);
			TimerLoadSet(TIMER3_BASE, TIMER_A, (SysCtlClockGet()*5/1000) -1);
			IntEnable(INT_TIMER3A);	//Enable interrupts
			TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
			IntMasterEnable();
			TimerEnable(TIMER3_BASE, TIMER_A);
		}

// 	else if (portd_edge_status == 2)
//		{
//			dist=0;
//		}



	}
}

void Timer3IntHandler(void)
{
	TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
	timer1_start=0;
	timer1_stop=0;
	portd_edge_status=0;
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet()*10) -1);
	IntEnable(INT_TIMER1A);	//Enable interrupts
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER1_BASE, TIMER_A);



	GPIOIntRegister(GPIO_PORTA_BASE, PortAIntHandler);
	GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);
	portd_edge_status=0; // set edge_status to rising
	GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2,4);
	//delay(10);
	SysCtlDelay(500);
	GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2,0);
}

void sonic_start_thread(void)
{
	char outstr[100];
	usbStringPut("Sonic read called");
	UARTCharPut(UART0_BASE,0x0D);
	UARTCharPut(UART0_BASE,0x0A);
	timer1_start=0;
	timer1_stop=0;
	sonic_cont=1;


	// Enable and Start Timer1A
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet()*10) -1);
	IntEnable(INT_TIMER1A);	//Enable interrupts
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER1_BASE, TIMER_A);



	GPIOIntRegister(GPIO_PORTA_BASE, PortAIntHandler);
	GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);
	portd_edge_status=0; // set edge_status to rising
	GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2,4);
	//delay(10);
	SysCtlDelay(500);
	GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2,0);
}


unsigned int get_sonic_dist()
{
	return dist;
}


//*********************************************************************************************
//
// This function will read the distance from the sonic sensor.
//
//*********************************************************************************************

unsigned int sonic_read(void)
{
	char outstr[100];
	usbStringPut("Sonic read called");
	UARTCharPut(UART0_BASE,0x0D);
	UARTCharPut(UART0_BASE,0x0A);
	timer1_start=0;
	timer1_stop=0;
	sonic_cont=0;


	// Enable and Start Timer1A
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet()*10) -1);
	IntEnable(INT_TIMER1A);	//Enable interrupts
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER1_BASE, TIMER_A);



	GPIOIntRegister(GPIO_PORTA_BASE, PortAIntHandler);
	GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);
	portd_edge_status=0; // set edge_status to rising
	GPIOIntEnable(GPIO_PORTA_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2,4);
	//delay(10);
	SysCtlDelay(500);
	GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_2,0);
	while (portd_edge_status != 2)
	{
		//sprintf(out,"portd_edge_status=%u",portd_edge_status);
//		usbStringPut("edge_status=");
//		ltoa( (long int) portd_edge_status,outstr);
//		usbStringPut(outstr);
//		UARTCharPut(UART0_BASE,0x0D);
//		UARTCharPut(UART0_BASE,0x0A);
//		SysCtlDelay(10000000);


	}
	return (timer1_start -  timer1_stop);
}




void sonic_pid(void)
{
	myStringPut("sonic_pid() called");
	myNLPut();
	sonic_start_thread();
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER2_BASE, TIMER_A, (SysCtlClockGet()/10) -1);
	IntEnable(INT_TIMER2A);	//Enable interrupts
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER2_BASE, TIMER_A);
	motor_left_forward();
	motor_right_forward();
	motor_left_duty(200);
	motor_right_duty(200);
//	while(1)
//	{
//		//dist = sonic_read();
//	}
}


void Timer2IntHandler(void)
{
	TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER2_BASE, TIMER_A, (SysCtlClockGet()/10) -1);
	IntEnable(INT_TIMER2A);	//Enable interrupts
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	IntMasterEnable();
	TimerEnable(TIMER2_BASE, TIMER_A);


	//
	myStringPut("Timer2IntHandler  ");
	int e;

	e=10*((SONIC_PID_SETPOINT)-get_sonic_dist());
	ltoa( (long int) get_sonic_dist(),out2);
	myStringPut(", dist=");
	myStringPut(out2);
	ltoa( (long int) e/10,out2);
	myStringPut(", error=");
	myStringPut(out2);
	myStringPut("; left_PW=");
	ltoa( (long int) get_motor_left_duty(),out2);
	myStringPut(out2);
	myStringPut("; right_PW=");
	ltoa( (long int) get_motor_right_duty(),out2);
	myStringPut(out2);
	myNLPut();

	// assume maximum error=70,000
	// max delta = 50

	// if e > 0 turn left LEFT=50 RIGHT=50+delta
	if (e > 0)
	{
		motor_left_duty(300);
		motor_right_duty(300+(e/700));
	}
	else if (e < 0)
	{
		// if e < 0 turn right LEFT=50+delta, RIGHT=50
		motor_left_duty(300-(e/700) );
		motor_right_duty(300);

	}
	else
	{
		motor_left_duty(300);
		motor_right_duty(300);
	}




	// if e == 0, straight ahead LEFT=RIGHT=50



}


//*********************************************************************************************
//
// This function will enable the color sensor. PB2 is used as OE (active low), PB3 is input from
// color sensor, PB6 is and 7 are color selects (S2 and S3 respectively)
//
//*********************************************************************************************

void color_init(void)
{



//	SysCtlPeripheralEnable(GPIO_PORTB_BASE);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlDelay(300);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_6 | GPIO_PIN_7);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4|GPIO_PIN_5);
	GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, GPIO_PIN_1|GPIO_PIN_0);
	SysCtlDelay(300);
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2,0); //Active low for OE
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6,64);
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,128);
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4|GPIO_PIN_5,0);
	GPIOPinWrite(GPIO_PORTD_BASE,GPIO_PIN_1|GPIO_PIN_0,GPIO_PIN_1);  // Set color sensor to 2% frequency scaling
	GPIOPadConfigSet(GPIO_PORTB_BASE,GPIO_PIN_4|GPIO_PIN_5,GPIO_STRENGTH_8MA,GPIO_PIN_TYPE_STD);
}


//*********************************************************************************************
//
//  PortBIntHandler is the color sensor frequency-counting ISR
//
//*********************************************************************************************

void PortBIntHandler( void )
{
	usbStringPut("PortBIntHandler()");
	GPIOIntClear(GPIO_PORTB_BASE,GPIO_INT_PIN_3);
	color_counter++;

}

//*********************************************************************************************
//
// This function turns ON COLOR sensing white LEDs
//
//*********************************************************************************************

void color_turn_on_led(void)
{

	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_4|GPIO_PIN_5);



}
//*********************************************************************************************
//
// This function turns OFF COLOR sensing white LEDs
//
//*********************************************************************************************
void color_turn_off_led(void)
{
	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_4|GPIO_PIN_5,0);
}
//*********************************************************************************************
//
// This function will call a function to read from the color sensor.
//
//*********************************************************************************************

unsigned int color_read(char color)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); //Enable Timer
	timer1_action = COLOR_ACTION; // set behavior of timer1 ISR
	color_flag = 0;
	color_counter = 0;

	switch( color)
	{
	case RED :
		GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6 | GPIO_PIN_7,0);
		break;
	case BLUE:
		GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6 ,0);
		GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,128);
		break;
	case CLEAR:
		GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6,64);
		GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_7,0);
		break;
	case GREEN:
		GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6|GPIO_PIN_7 ,64+128);
		break;
	default:
		return -1;
	}


	GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2,0); // set OE to ACTIVE LOW





	// Enable and Start Timer1A
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet()*1) -1);
	IntEnable(INT_TIMER1A);	//Enable interrupts
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER1_BASE, TIMER_A);
	GPIOIntRegister(GPIO_PORTB_BASE, PortBIntHandler);
	GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_RISING_EDGE);
	GPIOIntEnable(GPIO_PORTB_BASE, GPIO_PIN_3);
	IntMasterEnable();

	while( color_flag == 0)
	{

	}

	return color_counter;






}

unsigned int color_detect()
{
	unsigned int blue_value,red_value,clear_value,green_value;

	blue_value = color_read(BLUE);
	delay(1000);
	red_value = color_read(RED);
	delay(1000);
	clear_value=color_read(CLEAR);
	delay(1000);
	green_value=color_read(GREEN);

	return red_value;



}
