
#ifndef SENSORS_H_
#define SENSORS_H_

#define RED 0
#define BLUE 1
#define CLEAR 2
#define GREEN 3


extern void PortDIntHandler(void);
extern void Timer1IntHandler(void);
extern void Timer2IntHandler(void);
extern void Timer3IntHandler(void);
extern void usbStringPut(char * cptr);
extern void line_init(void);
extern unsigned int line_read(void);
extern char line_status(void);
extern void distance_init();
extern void sonic_init(void);
extern unsigned int sonic_read(void);
extern void sonic_start_thread(void);
extern void sonic_pid(void);


extern void color_init(void);
extern void color_turn_on_led(void);
extern void color_turn_off_led(void);
extern unsigned int color_read(char color);
extern unsigned int color_detect();
extern unsigned int get_sonic_dist();

#endif /* SENSORS_H_ */
