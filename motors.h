//**************************************************************************
//
//This file contains all of the functions defines necessary for operation of the
//robot motors.
//
//Author: Jerrod McClure
//
//***************************************************************************


#ifndef MOTORS_H_
#define MOTORS_H_

extern void motor_left_duty(unsigned int duty);
extern void motor_right_duty(unsigned int duty);
extern unsigned int get_motor_left_duty();
extern unsigned int get_motor_right_duty();
extern void motor_init(void);
extern void motor_left_forward(void);
extern void motor_right_forward(void);
extern void motor_left_stop (void);
extern void motor_right_stop (void);


#endif
