
#ifndef MISC_H_
#define MISC_H_

extern void Timer0IntHandler(void);
extern void myCharPut(char c);
extern void myStringPut(char * cptr);
extern char myCharGet(void);
extern bool myCharsAvail(void);
extern unsigned int char2Int( char ch);
extern void myBT_Init(void);
extern void myNLPut(void);
extern void myLEDInit(void);
extern void delay(unsigned int countus);
extern void usbStringPut(char * cptr);
extern void uint2str(unsigned int val, char * sout);

#endif /* MISC_H_ */
