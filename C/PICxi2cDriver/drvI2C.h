
#ifndef DRVI2C_H
#define DRVI2C_H

// CP uses i2c 1, address is 0x10
#define CPI2CCON    I2C1CON
#define CPCONbits   I2C1CONbits
#define CPI2CSTAT   I2C1STAT
#define CPSTATbits  I2C1STATbits
#define CPI2CMSK     I2C1MSK
#define CPI2CRCV     I2C1RCV
#define CPI2CTRN     I2C1TRN
#define CPI2CADD     I2C1ADD
#define CPI2CBRG    I2C1BRG

#define TRIS_SCK    TRISAbits.TRISA2
#define TRIS_SDA    TRISAbits.TRISA3

// 24LC16BT has a special addressing scheme,
// Slave_Address (7 bit) = 1010xxx. xxx define the EEPROM block addressed
// This means that the first 4 bits define the EEPROM address, an no other device on the chain can have an address
// starting with 0xA
// for now just use block 0, we get 256 bytes with this
#define C_CP_READ   0xA1
#define C_CP_WRITE  0xA0



BOOL PutAppleCPToSleep(void);
void drvI2CInit(void);






void drvI2CReset(void);
BOOL drvI2CReadRegister(UINT8 reg, UINT8* rxPtr);
BOOL CPWriteReg( UINT8 reg, UINT8 data);
BOOL CPWriteRegLoop( UINT8 reg, UINT8* data, UINT8 len, UINT8 slave_adr);
BOOL drvI2CReadRegisters(UINT8 reg, UINT8* rxPtr, UINT8 len, UINT8 slave_adr);
void drvI2CDisable(void);
BOOL drvI2CWriteRegisters(UINT8 reg, UINT8* data, UINT8 len, UINT8 slave_adr);
BOOL drvI2CWriteByte(UINT8 reg, UINT8 byte, UINT8 slave_adr );
BOOL PutAppleCPToSleep(void);
#endif
