

#ifndef TSENS_H
#define TSENS_H

#define TS_DEBUG 1
// I2C uses i2c 1, address is 0x10
#define I2CCON1    SSP1CON1
#define I2CCON1bits   SSP1CON1bits
#define I2CCON2    SSP1CON2
#define I2CCON2bits   SSP1CON2bits
#define I2CCON3    SSP1CON3
#define I2CCON3bits   SSP1CON3bits
#define I2CSTAT    SSP1STAT
#define I2CSTATbits   SSP1STATbits
#define I2CMSK     I2C1MSK
#define I2CRCV     SSP1BUF
#define I2CTRN     SSP1BUF
#define I2CBRG     SSP1ADD
#define I2CINTFLAG    PIR1bits.SSP1IF
#define I2CBCLFLAG    PIR2bits.BCL1IF

#define C_TS_ADDR   0x5A
#define C_TS_READ   0xB5
#define C_TS_WRITE  0xB4

// Device Registers - RAM 0x000xxxxx
#define TS_RAM_IR1_RAW                 0x04
#define TS_RAM_IR2_RAW                 0x05
#define TS_RAM_T_AMBEINT               0x06
#define TS_RAM_T_OBJ1                  0x07
#define TS_RAM_T_OBJ2                  0x08
// Deviec registers - Special
#define TS_ENTER_SLEEP              0xFF
#define TS_FLAG_REGISTER            0xF0
#define M_EE_BUSY_BIT               0x80
#define M_EE_DEAD_BIT               0x20
#define M_POR_BIT                   0x10
// Device Registers - EEPROM 0x001xxxxx
#define TS_EEPROM_TO_MAX                    (UINT8) ((0x00) | (0x20))
#define TS_EEPROM_TO_MIN                    (UINT8) ((0x01) | (0x20))
#define TS_EEPROM_PWMCTL                    (UINT8) ((0x02) | (0x20))
#define TS_EEPROM_TA_RANGE                  (UINT8) ((0x03) | (0x20))
#define TS_EEPROM_EMMISIVITY                (UINT8) ((0x04) | (0x20))
#define TS_EEPROM_CONFIG1                   (UINT8) ((0x05) | (0x20))
#define TS_EEPROM_SMB_ADDRESS               (UINT8) ((0x0E) | (0x20))
#define TS_EEPROM_ID1                (UINT8) ((0x1C) | (0x20)) //0x3C
#define TS_EEPROM_ID2                (UINT8) ((0x1D) | (0x20)) //0x3D
#define TS_EEPROM_ID3                (UINT8) ((0x1E) | (0x20)) //0x3E
#define TS_EEPROM_ID4                (UINT8) ((0x1F) | (0x20)) //0x3F

#define TS_CONFIG_MASK_LOW               0xF8 // For now only allow modification of FIR/IIR
#define TS_CONFIG_MASK_HIGH              0xF8


void TSCloseI2C(void) ;

void GetTempData(UINT8*dataPtr);

void I2CReset(void);
BOOL I2CReadRegister(UINT8 reg, UINT8* rxPtr);
BOOL I2CWriteReg(UINT8 reg, UINT8 data);
BOOL I2CWriteRegLoop(UINT8 reg, UINT8* data, UINT8 len);
BOOL I2CReadRegisterLoop(UINT8 reg, UINT8* rxPtr, UINT8 len);
void TSI2CTest(void);
void TSWriteEEPROM(UINT8 reg, UINT16 value);
void TSWriteConfigRegister(UINT8* data_ptr);

#endif
