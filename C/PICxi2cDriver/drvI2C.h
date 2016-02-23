
#ifndef DRVI2C_H
#define DRVI2C_H

// Change below defines to I2C2xxx if using I2C2
#define I2CCON        I2C1CON
#define I2CCONbits    I2C1CONbits
#define I2CSTAT       I2C1STAT
#define I2CSTATbits   I2C1STATbits
#define I2CMSK        I2C1MSK
#define I2CRCV        I2C1RCV
#define I2CTRN        I2C1TRN
#define I2CADD        I2C1ADD
#define I2CBRG        I2C1BRG

#define FCL        400000 // Check the max speed supported by your peripheral!!

#define I2C_DEBUG 0 // Change to 1 for debug messages


void drvI2CInit(void);
BOOL drvI2CReadRegisters(UINT8 reg, UINT8* rxPtr, UINT8 len, UINT8 slave_adr);
BOOL drvI2CWriteRegisters(UINT8 reg, UINT8* data, UINT8 len, UINT8 slave_adr);
BOOL drvI2CWriteByte(UINT8 reg, UINT8 byte, UINT8 slave_adr );

#endif
