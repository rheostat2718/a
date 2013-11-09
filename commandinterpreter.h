
#ifndef COMMANDINTERPRETER_H_
#define COMMANDINTERPRETER_H_

extern void myCmdLineInit(void);
extern void myCmdLine(void);
extern void cmdHello(void);
extern void cmdClock(void);
extern void cmdBlink(void);
extern void cmdStopBlink(void);
extern void cmdTurnOn(void);
extern void cmdTurnOff(void);
extern void cmdHelp(void);
extern void cmdforward(void);
extern void cmdstop(void);
extern void cmdslow(void);
extern void cmddistance(void);
extern void cmdline(void);
extern void cmdBangBang(void);
extern void cmdSonicRead(void);
extern void cmdSonicPoll(void);
extern void cmdStartPID(void);
extern void cmdSonicSample(void);

#endif /* COMMANDINTERPRETER_H_ */
