

#ifndef IPACP_H
#define IPACP_H

// CP uses i2c 1, address is 0x10
#define CPI2CCON1    SSP1CON1
#define CPCON1bits   SSP1CON1bits
#define CPI2CCON2    SSP1CON2
#define CPCON2bits   SSP1CON2bits
#define CPI2CCON3    SSP1CON3
#define CPCON3bits   SSP1CON3bits
#define CPI2CSTAT   SSP1STAT
#define CPSTATbits  SSP1STATbits
#define CPI2CMSK     I2C1MSK
#define CPI2CRCV     SSP1BUF
#define CPI2CTRN     SSP1BUF
#define CPI2CBRG    SSP1ADD
#define CPINTFLAG   PIR1bits.SSP1IF
#define CPBCLFLAG   PIR2bits.BCL1IF

#define C_CP_ADDR   0x10
#define C_CP_READ   0x51//0x21
#define C_CP_WRITE  0x50//0x20

// Device registers
#define FC_INFO_REG                 0x00 // Contents fixed = 0x12 or decimal 18
// ... define other registers here..







void iApCPReset(void);
BOOL iApCPReadRegister(UINT8 reg, UINT8* rxPtr);
BOOL CPWriteReg( UINT8 reg, UINT8 data);
BOOL CPWriteRegLoop( UINT8 reg, UINT8* data, UINT8 len);
BOOL iApCPReadRegisterLoop(UINT8 reg, UINT8* rxPtr, UINT8 len);
void CPTest(void);



#endif
