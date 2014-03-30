//-----------------------------------------------------------------------------
//
//  iPAcp.c
//
//
//  i2c driver for PIC18
//
//  Author: Govind Mukundan
//
//
//  ********************************* HISTORY ********************************
//
//  Version : 1.0
//  Date :
//  Description: Initial Version, tested with a i2c flash controller
//
//
//-----------------------------------------------------------------------------


#include "GenericTypeDefs.h"
#include "main.h"
#include "i2cDiverPic18.h"

#define USE_AND_OR
#include <i2c.h>

//NOTE: ACP registers are Big Endian, so you get the MSB at the last address, the iPhone knows this
// but if you are interpreting ACP data, you need to account for this
#define iAcp_DEBUG 0

#pragma udata section_TCPTest
UINT8 data[20];
#pragma udata

UINT8 FCIntStatus;

static void CPI2CIdle(void);
BOOL CPI2CStart(void);
void CPI2CIdle(void);
void CPI2CStop(void);
BOOL CPI2CSendByte(BYTE data);
void XDConfigure(void);


// Auth Co-processor section

void iApCPReset(void) {

    UINT16 temp = 1000;

    // Configure i2c interface
    SDA_TRIS = 1;
    SCL_TRIS = 1;
    ANSELC = 0;
    CPI2CCON1 = 0; // reset bits to 0
    CPI2CCON2 = 0;
    CPCON1bits.SSPEN = 0; // disable module
    CPI2CBRG = 0x13; //400k for 8M Fcy
    CPI2CSTAT = 0;
    CPSTATbits.SMP = 1;
    CPCON1bits.SSPM = 0x08; // master mode clock set by sspadd
    //CPCONbits.I2CSIDL = 1; // dont operate in idle mode
    //CPCONbits.SCLREL = 1;
    CPCON1bits.SSPEN = 1; // enable module
    temp = CPI2CRCV; // clear RBF flag
}

//Resets the I2C bus to Idle

void reset_i2c_bus(void) {
    int x = 0;

    //initiate stop bit
    CPCON2bits.PEN = 1;

    //wait for hardware clear of stop bit
    while (CPCON2bits.PEN) {
        //Delay10us(1);
        x++;
        if (x > 1) break;
    }
    CPCON2bits.RCEN = 0;
    // IFS1bits.MI2C1IF = 0; // Clear Interrupt
    CPCON1bits.WCOL = 0;
    //CPSTATbits.BCL = 0;

}

void CPI2CIdle(void) {
    UINT8 timeout = 100;
    /* Wait until I2C Bus is Inactive */
    while (CPCON2bits.SEN || CPCON2bits.PEN || CPCON2bits.RCEN ||
            CPCON2bits.RSEN || CPCON2bits.ACKEN || CPSTATbits.BF || (timeout--) );
}

BOOL CPI2CStart(void) {
    CPINTFLAG = 0;
    // wait for module idle
    CPI2CIdle();
    // Enable the Start condition
    CPCON2bits.SEN = 1;
    // Check for collisions
    if (CPCON1bits.WCOL) {
        return (FALSE);
    } else {
        // wait for module idle
        CPI2CIdle();
        return (TRUE);
    }
}

void CPI2CStop(void) {

    int x = 0;
    CPINTFLAG = 0;
    CPBCLFLAG = 0;
    // wait for module idle
    CPI2CIdle();
    //initiate stop bit
    CPCON2bits.PEN = 1;

    //wait for hardware clear of stop bit
    while (CPCON2bits.PEN) {
        // Delay10us(1);
        x++;
        if (x > 1) break;
    }
    CPCON2bits.RCEN = 0;
    // IFS1bits.MI2C1IF = 0; // Clear Interrupt
    CPCON1bits.WCOL = 0;
    //  CPSTATbits.BCL = 0;
    // wait for module idle
    CPI2CIdle();
}

BOOL CPI2CSendByte(BYTE data) {
    //while(CPSTATbits.TBF); //Wait till data is transmitted.
    CPINTFLAG = 0;
    // Send the byte
    CPI2CTRN = data;

    // Check for collisions
    if ((CPCON1bits.WCOL == 1)) {

        return (FALSE);
    } else {
        while (CPSTATbits.BF); // wait until write cycle is complete
        if ((CPCON1bits.WCOL == 1)) {

            return (FALSE);
        } else {
            // wait for module idle
            CPI2CIdle();
            return (TRUE);
        }
    }

}

BOOL iApCPReadRegister(UINT8 reg, UINT8* rxPtr) {
    //1. i2c start
    CPI2CStart();

    //2. Set CP in W Mode
    CPI2CSendByte(C_CP_WRITE);

    //3. Check ACK
    CPI2CIdle();
    if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
    {
        // Means the write was fine
        //4.Now send the register number you want to read
        CPI2CSendByte(reg);

        //5.Issue a repeated start = a start cond without a prior stop
        CPI2CStart();

        //6. Set CP in R mode
        CPI2CSendByte(C_CP_READ);

        //7. check for ACK
        CPI2CIdle();
        if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            CPCON2bits.RCEN = 1; // enable master read
            while (CPCON2bits.RCEN); // wait for byte to be received !(CPSTATbits.RBF)
            *rxPtr = CPI2CRCV;
            // create a stop cond
            CPI2CIdle();
            // CPSTATbits.I2COV = 0;


            //set ACK to high
            CPCON2bits.ACKDT = 1; // send nack
            CPCON2bits.ACKEN = 1;
            CPI2CStop();

            return TRUE;
        }
    }
    return FALSE;
}


BOOL iApCPReadRegisterLoop(UINT8 reg, UINT8* rxPtr, UINT8 len) {

    int value;
    UINT8 i, flag, ret, j;
    flag = 0;
    ret = FALSE;
#if(iAcp_DEBUG == 1)
    UART2PrintString("\r\n-----Start read reg:-----\r\n");
    UART2PutHex(reg);
    UART2PrintString("\r\n");
#endif
    for (i = 0; i < 100; i++) // wait for ACK for some time
    {
        //1. i2c start
        CPI2CStart();
        //CPI2CIdle();
        //2. Set CP in W Mode
        CPI2CSendByte(C_CP_WRITE);
        //3. Check ACK
        CPI2CIdle();
        if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }

    }
    // 4.if write cmd was successful, put the regno on the bus
    if (flag == 1) {
        flag = 0;
        CPI2CSendByte(reg);
        CPI2CIdle();
        if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            for (i = 0; i < len; i++) //read all the bytes
            {
                for (j = 0; j < 100; j++) {
                    //5.Issue a repeated start = a start cond without a prior stop
                    CPI2CStart();
                    //CPI2CIdle();
                    //6. Set CP in R mode
                    CPI2CSendByte(C_CP_READ);
                    //7. Check ACK
                    if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
                    {
                        flag = 1;
                        break;
                    }

                }
                if (flag == 1) {
                    flag = 0;

                    // got the ack, processor is in read mode
                    //8. read the register
                    CPCON2bits.RCEN = 1; // enable master read
                    while (CPCON2bits.RCEN); // wait for byte to be received !(CPSTATbits.RBF)
                    // Note that when you receive a byte the buffer full flag .RBF will be set
                    // So a check for Idle will fail at this point
                    // The flag is cleared only after the buffer has been read by the SW
                    *(rxPtr + i) = CPI2CRCV;
                    CPI2CIdle();
                    // CPSTATbits.I2COV = 0;
                    //

                    //9. Generate a NACK
                    CPCON2bits.ACKDT = 1; // send nack
                    CPCON2bits.ACKEN = 1;

                    //10. generate a stop
                    CPI2CStop();

                    ret = TRUE;
                } else {

                    ret = FALSE;
                }
            }
#if(iAcp_DEBUG == 1)
            for (i = 0; i < len; i++) {
                UART2PutChar('$');
                UART2PutHex(*(rxPtr + i));
            }
#endif
        } else {
            // UART2PrintString("Error NACK Rxed\r\n");

        }
    }
    CPI2CStop();
    // UART2PrintString("\r\nCON\r\n");
    // UART2PutHexWord(CPI2CCON);
    // UART2PrintString("\r\nSTAT\r\n");
    // UART2PutHexWord(CPI2CSTAT);
    return ret;
}

BOOL CPWriteReg(UINT8 reg, UINT8 data) {
    UINT8 i, flag;
    flag = 0;
    for (i = 0; i < 100; i++) {
        //1. i2c start
        CPI2CStart();
        //2. Set CP in W Mode
        CPI2CSendByte(C_CP_WRITE);
        //3. Check ACK
        CPI2CIdle();
        if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }

    }
    // 4.if write cmd was successful, put the data on the bus
    if (flag == 1) {
        CPI2CSendByte(reg);
        CPI2CIdle();
        if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            CPI2CSendByte(data);

            //UART2PrintString("ACK\r\n");
        } else {
            // UART2PrintString("Error NACK Rxed\r\n");
            return FALSE;
        }
        CPI2CStop();
        //  UART2PrintString("\r\nCON\r\n");
        //  UART2PutHexWord(CPI2CCON);
        //  UART2PrintString("\r\nSTAT\r\n");
        //   UART2PutHexWord(CPI2CSTAT);
        //   UART2PrintString("Write over\r\n");
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL CPWriteRegLoop(UINT8 reg, UINT8* data, UINT8 len) {
    UINT8 i, flag, j;
    flag = 0;
    for (i = 0; i < 100; i++) {
        //1. i2c start
        CPI2CStart();
        //2. Set CP in W Mode
        CPI2CSendByte(C_CP_WRITE);
        //3. Check ACK
        CPI2CIdle();
        if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }
        // UART2PutChar('.');
    }
    // 4.if write cmd was successful, put the data on the bus
    if (flag == 1) {
        CPI2CSendByte(reg);
        CPI2CIdle();
        for (j = 0; j < len; j++) {
            if (CPCON2bits.ACKSTAT == 0) // Did we receive an ACK ?
            {
                CPI2CSendByte(*(data + j));
            } else {
                //   UART2PrintString("Error NACK Rxed\r\n");
                return FALSE;
            }
        }
        CPI2CStop();
        return TRUE;
    } else {
        return FALSE;
    }
}

void CPTest(void) {
    // read out registers
    iApCPReadRegisterLoop(FC_INFO_REG, &data[0], 1);
}




