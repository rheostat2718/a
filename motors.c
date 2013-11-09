//**************************************************************************
//
//This file contains all of the functions necessary for operation of the
//robot motors.
//
//***************************************************************************
#define PART_TM4C123GH6PM 1 // This defines the current processor used. Necessary to use pin out macros.
#define PWM_FREQUENCY 50

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
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include <string.h>
#include "driverlib/pwm.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

// left motor duty
static unsigned int left_motor_duty;

// right motor duty
static unsigned int right_motor_duty;



static uint32_t ui32Load;

unsigned int get_motor_left_duty()
{
	return left_motor_duty;
}

unsigned int get_motor_right_duty()
{
	return right_motor_duty;
}


//**************************************************************************
//
//This function initialized the GPIO pins used to control motor function. It
//assumes Port E is being used. It makes use of pins PE1, PE2, and PE3. The pins used for the PWM
//are PE4 and PE5. It initializes the motors at stop.
//
//***************************************************************************

void motor_init(void)
{
	volatile uint32_t ui32PWMClock;
	volatile uint32_t ui32AdjustLeft, ui32AdjustRight;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); // Configure motor direction pins for both left and right
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 0);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0);

	left_motor_duty=50;
	right_motor_duty=50;

	ui32AdjustLeft= 10*left_motor_duty; // Set duty to 10%
	ui32AdjustRight = ui32AdjustLeft;


	SysCtlPWMClockSet(SYSCTL_PWMDIV_64); // Enable PWM click
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); // Enable PWM module 1.

	GPIOPinTypePWM(GPIO_PORTE_BASE, GPIO_PIN_4|GPIO_PIN_5); // Configure pins for PWM output.
	GPIOPinConfigure(GPIO_PE4_M1PWM2);
	GPIOPinConfigure(GPIO_PE5_M1PWM3);

	ui32PWMClock = SysCtlClockGet() / 64; // Acquire value for the PWM clock, assuming divisor 64.
	ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1; // Determine load

	PWMGenConfigure(PWM1_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN);
	PWMGenPeriodSet(PWM1_BASE, PWM_GEN_1, ui32Load);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2, ui32AdjustLeft * ui32Load / 1000);
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3, ui32AdjustRight * ui32Load / 1000);
	PWMOutputState(PWM1_BASE, PWM_OUT_2_BIT|PWM_OUT_3_BIT, true);
	PWMGenEnable(PWM1_BASE, PWM_GEN_1);

}

//**************************************************************************
//
// This function adjusted the duty of the left motor, accepting a value of
// 1-100.
//
//***************************************************************************

void motor_left_duty(unsigned int duty)
{
//	while (  ((duty-left_motor_duty) >= 10 ) || ( (left_motor_duty-duty) >= 10 ) )
//	{
//		if( duty > left_motor_duty)
//		{
//			left_motor_duty+=10;
//		}
//		else
//		{
//			left_motor_duty-=10;
//		}
//		//left_motor_duty = left_motor_duty + ( duty > left_motor_duty) ? 10 : -10;
//		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2, (10*left_motor_duty) * ui32Load / 1000);
//		SysCtlDelay(50000 *1000); //wait 1000 ms
//
//	}

	if (duty > 1000)
	{
		duty = 1000;
	}

	left_motor_duty=duty;
	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_2, (1*duty) * ui32Load / 1000);
}

//**************************************************************************
//
// This function adjusted the duty of the right motor, accepting a value of
// 1-100.
//
//***************************************************************************

void motor_right_duty(unsigned int duty)
{
//		while (  ((duty-left_motor_duty) > 10 ) || ( (left_motor_duty-duty) > 10 ) )
//		{
//			if( duty > left_motor_duty)
//			{
//				right_motor_duty+=10;
//			}
//			else
//			{
//				right_motor_duty-=10;
//			}
//			//left_motor_duty = left_motor_duty + ( duty > left_motor_duty) ? 10 : -10;
//			PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3, (10*right_motor_duty) * ui32Load / 1000);
//			SysCtlDelay(50000*1000); //wait 1000 ms
//
//		}
	if (duty > 1000)
	{
		duty = 1000;
	}
	right_motor_duty=duty;
		PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3, (1*duty) * ui32Load / 1000);
//	PWMPulseWidthSet(PWM1_BASE, PWM_OUT_3, (10*duty) * ui32Load / 1000);
}

//**************************************************************************
//
// The following functions set the direction of motor by means of the GPIO.
//
//***************************************************************************

void motor_left_stop(void) //Debug, delete later.
{
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 1);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 2);
}

void motor_right_stop(void) //Debug, delete later.
{
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 4);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 8);
}

void motor_left_forward(void) //Debug, delete later.
{
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 1);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_1, 0);
}

void motor_right_forward(void) //Debug, delete later.
{
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_2, 4);
	GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_3, 0);
}
