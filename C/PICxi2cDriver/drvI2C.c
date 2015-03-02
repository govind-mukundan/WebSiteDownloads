//-----------------------------------------------------------------------------
//
//  Generic i2c driver
//
//  Author: Govind Mukundan
//
//
//  ********************************* HISTORY ********************************
//
//  Version : 1.0
//  Date :
//  Description: Initial version for use with E100L RTC / EEPROM
//
//
//-----------------------------------------------------------------------------


//#include "C:\Microchip\Microchip\MPLAB C30\support\PIC24F\hp24FJ256GB106.h"
#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "HardwareProfile.h"
#include "drvI2C.h"
#include "uart2.h"
#define USE_AND_OR
#include <i2c.h>


#define I2C_DEBUG 0
#define FCL        400000 // Check the max speed supported by your peripheral!!
#define FCL_2_0B   40000

static BOOL I2CLocked; // Variable to keep track of the initialization. When MFI library takes control of I2C, its not available for EEPROM and RTC
static void CPI2CIdle(void);
static BOOL CPI2CStart(void);
static void CPI2CIdle(void);
static void CPI2CStop(void);
static BOOL CPI2CSendByte(BYTE data);

// Any other module taking control of the i2c pins must lock it to prevent conflicts with this driver

void drvI2CLock(void) {
    I2CLocked = TRUE;
}

void drvI2CRelease(void) {
    I2CLocked = FALSE;
}

void drvI2CInit(void) {
    UINT16 temp = 0;
    DelayMs(100);
    CPI2CCON = 0; // reset bits to 0
    CPCONbits.I2CEN = 0; // disable module
    CPI2CBRG = (SSMGetCurrentPeripheralClock() / FCL) - (SSMGetCurrentPeripheralClock() / 10000000) - 1; //Formula from datasheet
    //CPI2CBRG = ((SSMGetCurrentPeripheralClock() / (2 * Fsck)) - 1);
    CPI2CSTAT = 0;
    CPCONbits.I2CSIDL = 1; // dont operate in idle mode
    //CPCONbits.SCLREL = 1;
    CPCONbits.I2CEN = 1; // enable module
    temp = CPI2CRCV; // clear RBF flag
    UART2PrintString("Configured i2c1\n");
    I2CLocked = FALSE;

}



static void CPI2CIdle(void) {
    UINT8 t = 255;
    /* Wait until I2C Bus is Inactive */
    while (CPCONbits.SEN || CPCONbits.PEN || CPCONbits.RCEN ||
            CPCONbits.RSEN || CPCONbits.ACKEN || CPSTATbits.TRSTAT || t--);
}

static BOOL CPI2CStart(void) {
    // wait for module idle
    CPI2CIdle();
    // Enable the Start condition
    CPCONbits.SEN = 1;


    // Check for collisions
    if (CPSTATbits.BCL) {
        UART2PrintString("I2C Start Cond Error! \r\n");
        UART2PrintString("CON\n");
        UART2PutHexWord(CPI2CCON);

        UART2PrintString("STAT\n");
        UART2PutHexWord(CPI2CSTAT);
        return (FALSE);
    } else {
        // wait for module idle
        CPI2CIdle();
        return (TRUE);
    }
}

static void CPI2CStop(void) {
    int x = 0;
    // wait for module idle
    CPI2CIdle();
    //initiate stop bit
    CPCONbits.PEN = 1;

    //wait for hardware clear of stop bit
    while (CPCONbits.PEN) {
        Delay10us(1);
        x++;
        if (x > 1) break;
    }
    CPCONbits.RCEN = 0;
    // IFS1bits.MI2C1IF = 0; // Clear Interrupt
    CPSTATbits.IWCOL = 0;
    CPSTATbits.BCL = 0;
    // wait for module idle
    CPI2CIdle();
}

static BOOL CPI2CSendByte(BYTE data) {
    //while(CPSTATbits.TBF); //Wait till data is transmitted.
    // Send the byte
    CPI2CTRN = data;

    // Check for collisions
    if ((CPSTATbits.IWCOL == 1)) {
        UART2PrintString("IWOCL \n");
        UART2PrintString("CON\n");
        UART2PutHexWord(CPI2CCON);

        UART2PrintString("STAT\n");
        UART2PutHexWord(CPI2CSTAT);
        return (FALSE);
    } else {
        while (CPSTATbits.TRSTAT); // wait until write cycle is complete
        if ((CPSTATbits.BCL == 1)) {
            UART2PrintString("I2C Tx Error!\n");
            UART2PrintString("CON\n");
            UART2PutHexWord(CPI2CCON);

            UART2PrintString("STAT\n");
            UART2PutHexWord(CPI2CSTAT);
            return (FALSE);
        } else {
            // wait for module idle
            CPI2CIdle();
            return (TRUE);
        }
    }
}


/**
 * @brief Read data from an I2C slave
 *
 * This function can be used to read one or more sequential registers from a slave.
 * To read multiple registers, the slave must support multi-byte reads.
 * 
 * @param reg The register to start reading from (UINT8)
 * @param rxPtr A pointer to where the read data should be stored (UINT8*)
 * @param len Number of bytes/registers to read
 * @param slave_adr The 7 bit address of the slave without the R/W bits set
 * @return Boolean indicating if operation completed successfully or not
 */
BOOL drvI2CReadRegisters(UINT8 reg, UINT8* rxPtr, UINT8 len, UINT8 slave_adr) {

    UINT8 i, flag, ret, j;
    flag = 0;
    ret = FALSE;
#if(I2C_DEBUG == 1)
    UART2PrintString("\r\n-----Start read reg:-----\r\n");
    UART2PutHex(reg);
    UART2PrintString("\r\n");
#endif
    if (I2CLocked) return FALSE;
    for (i = 0; i < 100; i++) // wait for ACK for some time
    {
        //1. i2c start
        CPI2CStart();
        //2. Set Slave in W Mode
        CPI2CSendByte((slave_adr << 1) | 0);
        //3. Check ACK
        CPI2CIdle();
        if (CPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }
        UART2PutChar('.');
    }

    if (!flag) return FALSE; // Exit if there was a problem
    flag = 0;
    // 4.if write cmd was successful, put the regno on the bus
    CPI2CSendByte(reg);
    if (CPSTATbits.ACKSTAT != 0) // Did we receive an ACK ?
    {
        UART2PrintString("Error NACK Rxed\n");
        return FALSE; // Exit if there was a problem
    }
    // Now that the register addres is setup, we can ask the slave to enter read mode.
    for (j = 0; j < 100; j++) {
        //5.Issue a repeated start = a start cond without a prior stop
        CPI2CStart();
        //6. Set Slave in R mode
        CPI2CSendByte((slave_adr << 1) | 1);
        //7. Check ACK
        if (CPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }
        UART2PutChar('.');
    }

    if (!flag) return FALSE; // Exit if there was a problem

    for (i = 0; i < len; i++) //read all the bytes
    {
        CPI2CIdle();
        // got the ack, processor is in read mode
        //8. read the register
        CPCONbits.RCEN = 1; // enable master read
        while (CPCONbits.RCEN); // wait for byte to be received !(CPSTATbits.RBF)

        CPI2CIdle();
        CPSTATbits.I2COV = 0;
        *(rxPtr + i) = CPI2CRCV;

        if ((i + 1) == len) {
            //9. Generate a NACK on last byte
            CPCONbits.ACKDT = 1; // send nack
            CPCONbits.ACKEN = 1;
            //10. generate a stop
            CPI2CStop();
        } else {
            CPCONbits.ACKDT = 0; // send ACK for sequential reads
            CPCONbits.ACKEN = 1;
        }

        ret = TRUE;
    }
#if(I2C_DEBUG == 1)
    for (i = 0; i < len; i++) {
        UART2PutChar('$');
        UART2PutHex(*(rxPtr + i));
    }
#endif

    return ret;
}

/**
 * @brief Write data into an I2C slave
 *
 * This function can be used to write one or more sequential registers from a slave.
 * To write multiple registers, the slave must support multi-byte writes.
 *
 * @param reg The register to start writing to (UINT8)
 * @param rxPtr A pointer to where the data should be fetched from (UINT8*)
 * @param len Number of bytes/registers to write
 * @param slave_adr The 7 bit address of the slave without the R/W bits set
 * 
 * @return Boolean indicating if operation completed successfully or not
 */
BOOL drvI2CWriteRegisters(UINT8 adr, UINT8* data, UINT8 len, UINT8 slave_adr) {
    UINT8 i, flag, j;
    flag = 0;
    if (I2CLocked) return FALSE;
    for (i = 0; i < 100; i++) {
        //1. i2c start
        CPI2CStart();
        //2. Set CP in W Mode
        CPI2CSendByte((slave_adr << 1) | 0);
        //3. Check ACK
        CPI2CIdle();
        if (CPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }
        UART2PutChar('.');
    }

    if (!flag) return (FALSE); // Exit if there was a problem
    // 4.if write cmd was successful, put the adress on the bus
    CPI2CSendByte(adr);
    CPI2CIdle();
    for (j = 0; j < len; j++) {
        if (CPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            CPI2CSendByte(*(data + j));
        } else {
            UART2PrintString("Error NACK Rxed\n");
            return FALSE;
        }
    }
    CPI2CStop();

    return TRUE;

}

/**
 * @brief A wrapper around drvI2CWriteRegisters() to write only a byte of data
 *
 * @param reg The register to start reading from (UINT8)
 * @param byte The byte to write
 * @param slave_adr The 7 bit address of the slave without the R/W bits set
 * @return Boolean indicating if operation completed successfully or not
 */
BOOL drvI2CWriteByte(UINT8 reg, UINT8 byte, UINT8 slave_adr) {
    return ( drvI2CWriteRegisters(reg, &byte, 1, slave_adr));
}


