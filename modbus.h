/*
 * modbus.h
 *
 *  Created on: Oct 12, 2013
 *      Author: Jerrod
 */

#ifndef MODBUS_H_
#define MODBUS_H_

extern unsigned int checkSum(char * cptr);
extern char checkSumDigit0( unsigned int cs);
extern char checkSumDigit1( unsigned int cs);
extern char myModbusRead(char * modbus, char * cmdName0);

#endif /* MODBUS_H_ */

