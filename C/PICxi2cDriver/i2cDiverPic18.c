//-----------------------------------------------------------------------------
//
//  i2cDiverPic18.c
//
//
//  i2c driver for MLX90614 Temperature sensor
//
//  Author: Govind Mukundan
//
//
//  ********************************* HISTORY ********************************
//
//  Version : 1.0
//  Date :
//  Description: Initial Version
//
//
//-----------------------------------------------------------------------------



#include "GenericTypeDefs.h"
#include "i2cDiverPic18.h"
#define CRC8_POLY         0x07
#define CRC8_INIT_REM     0x00

static UINT8 data[20];
static BOOL Initialized;

UINT8 CalculateCRC8(UINT8 CRC, UINT8 Poly, UINT8 *Pmsg, UINT8 Msg_Size);


static void I2CIdle(void);
static BOOL I2CStart(void);
static void I2CIdle(void);
static void I2CStop(void);
static BOOL I2CSendByte(BYTE data);
static void reset_i2c_bus(void);

// Auth Co-processor section

void I2CReset(void) {

    UINT16 temp;
    PMD1bits.MSSP1MD = 0;
    // Configure i2c interface
    SDA_TRIS = 1;
    SCL_TRIS = 1;
    ANSELC = 0;
    I2CCON1 = 0; // reset bits to 0
    I2CCON2 = 0;
    I2CCON1bits.SSPEN = 0; // disable module
#if (I2C_GLOBAL_BUS_SPEED == I2C_100K)
    I2CBRG = 0x4F; //100k for 8M Fcy
#else
    //#error "Max SMB Bus speed is 100K"
    I2CBRG = 0x4F; //100k for 8M Fcy
#endif
    I2CSTAT = 0;
    I2CSTATbits.SMP = 1;
    I2CCON1bits.SSPM = 0x08; // master mode clock set by sspadd
    //I2CCONbits.I2CSIDL = 1; // dont operate in idle mode
    //I2CCONbits.SCLREL = 1;
    I2CCON1bits.SSPEN = 1; // enable module
    temp = I2CRCV; // clear RBF flag
    Initialized = TRUE;
}

//Resets the I2C bus to Idle

static void reset_i2c_bus(void) {
    int x = 0;

    //initiate stop bit
    I2CCON2bits.PEN = 1;

    //wait for hardware clear of stop bit
    while (I2CCON2bits.PEN) {
        //Delay10us(1);
        x++;
        if (x > 1) break;
    }
    I2CCON2bits.RCEN = 0;
    // IFS1bits.MI2C1IF = 0; // Clear Interrupt
    I2CCON1bits.WCOL = 0;
    //I2CSTATbits.BCL = 0;

}

static void I2CIdle(void) {
    UINT8 timeout = 100;
    /* Wait until I2C Bus is Inactive */
    while (I2CCON2bits.SEN || I2CCON2bits.PEN || I2CCON2bits.RCEN ||
            I2CCON2bits.RSEN || I2CCON2bits.ACKEN || I2CSTATbits.BF || (timeout--));
}

static BOOL I2CStart(void) {
    I2CINTFLAG = 0;
    // wait for module idle
    I2CIdle();
    // Enable the Start condition
    I2CCON2bits.SEN = 1;
    // Check for collisions
    if (I2CCON1bits.WCOL) {
        return (FALSE);
    } else {
        // wait for module idle
        I2CIdle();
        return (TRUE);
    }
}

static void I2CStop(void) {

    int x = 0;
    I2CINTFLAG = 0;
    I2CBCLFLAG = 0;
    // wait for module idle
    I2CIdle();
    //initiate stop bit
    I2CCON2bits.PEN = 1;

    //wait for hardware clear of stop bit
    while (I2CCON2bits.PEN) {
        // Delay10us(1);
        x++;
        if (x > 1) break;
    }
    I2CCON2bits.RCEN = 0;
    // IFS1bits.MI2C1IF = 0; // Clear Interrupt
    I2CCON1bits.WCOL = 0;
    //  I2CSTATbits.BCL = 0;
    // wait for module idle
    I2CIdle();
}

static BOOL I2CSendByte(BYTE data) {
    //while(I2CSTATbits.TBF); //Wait till data is transmitted.
    I2CINTFLAG = 0;
    // Send the byte
    I2CTRN = data;

    // Check for collisions
    if ((I2CCON1bits.WCOL == 1)) {

        return (FALSE);
    } else {
        while (I2CSTATbits.BF); // wait until write cycle is complete
        if ((I2CCON1bits.WCOL == 1)) {

            return (FALSE);
        } else {
            // wait for module idle
            I2CIdle();
            return (TRUE);
        }
    }

}

BOOL I2CReadRegister(UINT8 reg, UINT8* rxPtr) {
    //1. i2c start
    I2CStart();

    //2. Set I2C in W Mode
    I2CSendByte(C_TS_WRITE);

    //3. Check ACK
    I2CIdle();
    if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
    {
        // Means the write was fine
        //4.Now send the register number you want to read
        I2CSendByte(reg);

        //5.Issue a repeated start = a start cond without a prior stop
        I2CStart();

        //6. Set I2C in R mode
        I2CSendByte(C_TS_READ);

        //7. check for ACK
        I2CIdle();
        if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            I2CCON2bits.RCEN = 1; // enable master read
            while (I2CCON2bits.RCEN); // wait for byte to be received !(I2CSTATbits.RBF)
            *rxPtr = I2CRCV;
            // create a stop cond
            I2CIdle();
            // I2CSTATbits.I2COV = 0;


            //set ACK to high
            I2CCON2bits.ACKDT = 1; // send nack
            I2CCON2bits.ACKEN = 1;
            I2CStop();

            return TRUE;
        }
    }
    return FALSE;
}

BOOL I2CReadRegisterLoop(UINT8 reg, UINT8* rxPtr, UINT8 len) {
    UINT8 temp[6];
    UINT8 i, flag, ret, j;
    ret = FALSE;
    if (!Initialized)
        I2CReset();

#if(TS_DEBUG == 1)
    DebugPrintString("\r\n-----Start read reg:-----\r\n");
    DebugPutHex(reg);
    DebugPrintString("\r\n");
    temp[0] = C_TS_WRITE;
    temp[1] = reg;
    temp[2] = C_TS_READ;
#endif
    for (i = 0; i < 100; i++) // wait for ACK for some time
    {
        //1. i2c start
        I2CStart();
        //I2CIdle();
        //2. Set I2C in W Mode
        I2CSendByte(C_TS_WRITE);
        //3. Check ACK
        I2CIdle();
        if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }

    }
    // 4.if write cmd was successful, put the regno on the bus
    if (flag == 1) {
        flag = 0;
        I2CSendByte(reg);
        I2CIdle();
        if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            // Set into Read mode
            for (j = 0; j < 100; j++) {
                //5.Issue a repeated start = a start cond without a prior stop
                I2CStart();
                //I2CIdle();
                //6. Set I2C in R mode
                I2CSendByte(C_TS_READ);
                //7. Check ACK
                if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
                {
                    flag = 1;
                    break;
                }

            }
            if (flag == 1) {
                flag = 0;
                // Read out all the bytes continuously
                for (i = 0; i < len; i++) //read all the bytes
                {
                    // got the ack, processor is in read mode
                    //8. read the register
                    I2CCON2bits.RCEN = 1; // enable master read
                    while (I2CCON2bits.RCEN); // wait for byte to be received !(I2CSTATbits.RBF)
                    // Note that when you receive a byte the buffer full flag .RBF will be set
                    // So a check for Idle will fail at this point
                    // The flag is cleared only after the buffer has been read by the SW
                    *(rxPtr + i) = I2CRCV;
                    I2CIdle();

                    //9. Generate a ACK / NACK
                    if (i < len - 1)
                        I2CCON2bits.ACKDT = 0; // send ack
                    else
                        I2CCON2bits.ACKDT = 1; // send Nack

                    I2CCON2bits.ACKEN = 1;
                    I2CIdle();
                    ret = TRUE;

                }
#if(TS_DEBUG == 1)
                for (j = 0; j < len; j++) {
                    DebugPutChar('$');
                    DebugPutHex(*(rxPtr + j));
                    temp[3 + j] = *(rxPtr + j);
                }
                CalculateCRC8(0, CRC8_POLY, temp, 5); // Ignore CRC for now
#endif
            } else {

                ret = FALSE;
            }

        } else {
            // UART2PrintString("Error NACK Rxed\r\n");
        }
    }
    //10. generate a stop
    I2CStop();

    return ret;
}

BOOL I2CWriteReg(UINT8 reg, UINT8 data) {
    UINT8 i, flag;
    flag = 0;
    for (i = 0; i < 100; i++) {
        //1. i2c start
        I2CStart();
        //2. Set I2C in W Mode
        I2CSendByte(C_TS_WRITE);
        //3. Check ACK
        I2CIdle();
        if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }

    }
    // 4.if write cmd was successful, put the data on the bus
    if (flag == 1) {
        I2CSendByte(reg);
        I2CIdle();
        if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            I2CSendByte(data);

            //UART2PrintString("ACK\r\n");
        } else {
            // UART2PrintString("Error NACK Rxed\r\n");
            return FALSE;
        }
        I2CStop();
        //  UART2PrintString("\r\nCON\r\n");
        //  UART2PutHexWord(I2CCON);
        //  UART2PrintString("\r\nSTAT\r\n");
        //   UART2PutHexWord(I2CSTAT);
        //   UART2PrintString("Write over\r\n");
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL I2CWriteRegLoop(UINT8 reg, UINT8* data, UINT8 len) {
    UINT8 i, flag, j;
    flag = 0;
#if(TS_DEBUG == 1)
    DebugPrintString("\r\n-----Start Write reg:-----\r\n");
    DebugPutHex(reg);
    DebugPrintString("\r\n");
#endif
    for (i = 0; i < 100; i++) {
        //1. i2c start
        I2CStart();
        //2. Set I2C in W Mode
        I2CSendByte(C_TS_WRITE);
        //3. Check ACK
        I2CIdle();
        if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }
#if(TS_DEBUG == 1)
        DebugPutChar('.');
#endif
    }
    // 4.if write cmd was successful, put the data on the bus
    if (flag == 1) {
        I2CSendByte(reg);
        I2CIdle();
        for (j = 0; j < len; j++) {
            if (I2CCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
            {
                I2CSendByte(*(data + j));
#if(TS_DEBUG == 1)
                DebugPutChar('#');
                DebugPutHex(*(data + j));
#endif
            } else {
#if(TS_DEBUG == 1)
                DebugPrintString("Error NACK Rxed\r\n");
#endif
                return FALSE;
            }
        }
        I2CStop();
        return TRUE;
    } else {
        return FALSE;
    }
}

void TSI2CTest(void) {

    I2CReset();
    // read out registers
    I2CReadRegisterLoop(TS_RAM_IR1_RAW, &data[0], 3);
    I2CReadRegisterLoop(TS_RAM_IR2_RAW, &data[0], 3);
    I2CReadRegisterLoop(TS_RAM_T_AMBEINT, &data[0], 3);
    I2CReadRegisterLoop(TS_RAM_T_OBJ1, &data[0], 3);
    I2CReadRegisterLoop(TS_RAM_T_OBJ2, &data[0], 3);
    //I2CReadRegisterLoop(TS_FLAG_REGISTER, &data[0], 3);

    I2CReadRegisterLoop(TS_EEPROM_TO_MAX, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_TO_MIN, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_PWMCTL, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_TA_RANGE, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_EMMISIVITY, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_CONFIG1, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_SMB_ADDRESS, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_ID1, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_ID2, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_ID3, &data[0], 3);
    I2CReadRegisterLoop(TS_EEPROM_ID4, &data[0], 3);

}

//Will return - Ambient, Obj1, Raw1, Raw2. Obj2 is always 0. 2 bytes each
// So array must have at least 8 bytes. 10 is good for updard compatibility

void GetTempData(UINT8*dataPtr) {

    if (!Initialized)
        I2CReset();

    I2CReadRegisterLoop(TS_RAM_T_AMBEINT, &dataPtr[0], 3); // discard byte 3 coz its a checksum
    I2CReadRegisterLoop(TS_RAM_T_OBJ1, &dataPtr[2], 3);
    I2CReadRegisterLoop(TS_RAM_IR1_RAW, &dataPtr[4], 3);
    I2CReadRegisterLoop(TS_RAM_IR2_RAW, &dataPtr[6], 3);

}

void TSWriteEEPROM(UINT8 reg, UINT16 value) {
    // EEPROM location must first be written with 0x0000.
    // Make sure device is not busy, then write the new value and wait till busy flag is reset.
    // 2 bytes for address and reg, 2 for data and 1 for SMB checksum
    UINT8 param[2 + 2 + 1];

    if (!Initialized)
        I2CReset();

    param[0] = C_TS_WRITE;
    param[1] = reg;
    param[2] = 0;
    param[3] = 0;
    param[4] = CalculateCRC8(0, CRC8_POLY, param, 4);

    if (I2CWriteRegLoop(reg, &param[2], 3)) {
        DelayMs(50);
#if 0
        data[0] = M_EE_BUSY_BIT;
        while ((Z_BitIsSet(data[0], M_EE_BUSY_BIT)) && (!Z_BitIsSet(data[0], M_EE_DEAD_BIT))) {
            DelayMs(100);
            I2CReadRegisterLoop(TS_FLAG_REGISTER, &data[0], 3);
        }
#endif
        param[2] = Z_GetLowByteU16(value); // Low byte must be sent first
        param[3] = Z_GetHighByteU16(value);
        param[4] = CalculateCRC8(0, CRC8_POLY, param, 4);
        if (I2CWriteRegLoop(reg, &param[2], 3)) {
            DelayMs(50);
            // Power cycle the sensor
            TempSensorOFF();
            DelayMs(100);
            TempSensorON();
        }
    }

}

// Dataptr points to the LSB

void TSWriteConfigRegister(UINT8* data_ptr) {
    // EEPROM location must first be written with 0x0000.
    // Make sure device is not busy, then write the new value and wait till busy flag is reset.
    // 2 bytes for address and reg, 2 for data and 1 for SMB checksum
    // Not all bytes of config register can be written, unmodified part must be ORed
    UINT8 param[2 + 2 + 1];
    UINT8 datum[3];

    if (!Initialized)
        I2CReset();

    I2CReadRegisterLoop(TS_EEPROM_CONFIG1, &datum[0], 3);
#if(TS_DEBUG == 1)
    DebugPrintString("original=\r\n");
    DebugPutChar('#');
    DebugPutHex(datum[0]);
    DebugPutChar('#');
    DebugPutHex(datum[1]);
    DebugPrintString("New=\r\n");
    DebugPutChar('#');
    DebugPutHex(data_ptr[0]);
    DebugPutChar('#');
    DebugPutHex(data_ptr[1]);
#endif
    // Clear Fir and IIR bits
    datum[0] &= TS_CONFIG_MASK_LOW;
    datum[1] &= TS_CONFIG_MASK_HIGH;
    // clear all non FIR and IIR bits
    *data_ptr &= ~TS_CONFIG_MASK_LOW;
    *(data_ptr + 1) &= ~TS_CONFIG_MASK_LOW;
    // Add the new Fir and IIR values
    datum[0] |= *data_ptr;
    datum[1] |= *(data_ptr + 1);

#if(TS_DEBUG == 1)
    DebugPrintString("New=\r\n");
    DebugPutChar('#');
    DebugPutHex(data_ptr[0]);
    DebugPutChar('#');
    DebugPutHex(data_ptr[1]);
#endif

    // Clear register data
    param[0] = C_TS_WRITE;
    param[1] = TS_EEPROM_CONFIG1;
    param[2] = 0;
    param[3] = 0;
    param[4] = CalculateCRC8(0, CRC8_POLY, param, 4);

    if (I2CWriteRegLoop(TS_EEPROM_CONFIG1, &param[2], 3)) {
        DelayMs(50);

        param[2] = datum[0];
        param[3] = datum[1];
        param[4] = CalculateCRC8(0, CRC8_POLY, param, 4);
#if(TS_DEBUG == 1)
        DebugPrintString("Final=\r\n");
        DebugPutChar('#');
        DebugPutHex(param[2]);
        DebugPutChar('#');
        DebugPutHex(param[3]);
#endif
        // Write the new value
        if (I2CWriteRegLoop(TS_EEPROM_CONFIG1, &param[2], 3)) {
            DelayMs(50);
            // Power cycle the sensor
            TempSensorOFF();
            DelayMs(100);
            TempSensorON();
        }
    }

}

// Resource for common CRC algorithms -> http://reveng.sourceforge.net/crc-catalogue/all.htm

/*-----------------------------------------------------------------------------+
|unsigned short crc8MakeBitwise(...)
| IN: unsigned char CRC           => Initial crc value
|     unsigned char Poly          => CRC polynomial
|     unsigned int *Pmsg          => Pointer to input data stream
|     unsigned char Msg_Size      => No. of bytes
| OUT:unsigned short CRC          => Result of CRC function
 http://processors.wiki.ti.com/index.php/Implementing_SMBus_using_USCI
------------------------------------------------------------------------------*/
UINT8 CalculateCRC8(UINT8 CRC, UINT8 Poly, UINT8 *Pmsg, UINT8 Msg_Size) {
    UINT8 i, j, carry;
    UINT8 msg;

    CRC = *Pmsg++; // first byte loaded in "crc"
    for (i = 0; i < Msg_Size - 1; i++) {
        msg = *Pmsg++; // next byte loaded in "msg"

        for (j = 0; j < 8; j++) {
            carry = CRC & 0x80; // ckeck if MSB=1
            CRC = (CRC << 1) | (msg >> 7); // Shift 1 bit of next byte into crc
            if (carry) CRC ^= Poly; // If MSB = 1, perform XOR
            msg <<= 1; // Shift left msg byte by 1
        }
    }

    // The previous loop computes the CRC of the input bit stream. To this,
    // 8 trailing zeros are padded and the CRC of the resultant value is
    // computed. This gives the final CRC of the input bit stream.
    for (j = 0; j < 8; j++) {
        carry = CRC & 0x80;
        CRC <<= 1;
        if (carry) CRC ^= Poly;
    }

#if(TS_DEBUG == 1)
    DebugPutChar('@');
    DebugPutHex(CRC);
#endif
    return (CRC);
}

void TSCloseI2C(void) {
    Initialized = FALSE;
}

