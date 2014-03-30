/*******************************************************************************

Summary:
    This file contains the interface definition for the GoWorld i2c touch panel.

Description:
    
FileName:       drvTP.c
Owner:          Govind Mukundan

 Version History
//  Version : 1.0
//  Date : 26/07/12
//  Description: Initial Version, tested with PIC32MX795 and Explorer 16 Board

 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Includes
// *****************************************************************************
// *****************************************************************************
#include <GenericTypeDefs.h>
#include <plib.h>
#include "HardwareProfile.h"
#include <drvTP.h>
#include <drvUART.h>

/* NOTE: The PIC I2C engine cannot queue commands. So it is necessary to wait till the engine is idle
 * before issuing a new operation command. So you have to set the bit for a START condition, then
 * wait till the condition is generated (or the engine is idle) before sending the data and so on..
 */
UINT8 TpNoOfPoints;
UINT8 TpGestureID;
ts_TpDataPoint TpTouchPoint1;
ts_TpDataPoint TpTouchPoint2;
BOOL TPIntFlag;
void TPI2CIdle(void);
BOOL TPI2CStart(void);
void TPI2CStop(void);
BOOL TPI2CSendByte(BYTE data);
void I2CWait(unsigned int cnt);
BOOL iApTPReadRegisterLoop(UINT8 reg, UINT8* rxPtr, UINT8 len);
BOOL TPWriteRegLoop(UINT8 reg, UINT8* data, UINT8 len);
unsigned int MasterputsI2C(unsigned char * wrptr);
void TPi2cCommError(void);


#define iAcp_DEBUG 1

void drvInitTP(void)
{
    UINT16 temp = 0;
    TP_RESET_TRIS = 0;
    TP_RESET_LAT = 0;
    I2CWait(100);
    TP_RESET_LAT = 1;
    TP_INT_TRIS = 1;
    // Configure i2c interface
    TPI2CCON = 0; // reset bits to 0
    TPCONbits.I2CEN = 0; // disable module
    TPI2CBRG = I2C_BRG; //100k for 16M Fcy
    TPI2CSTAT = 0;
    TPCONbits.I2CSIDL = 1; // dont operate in idle mode
    //TPCONbits.SCLREL = 1;
    TPCONbits.I2CEN = 1; // enable module
    temp = TPI2CRCV; // clear RBF flag
    TPI2CIdle();
    drvUARTPrintString(_UART_LOG, "TP reset! \r\n");

    // Configure INT1 for falling
    INTEnable(INT_INT1, INT_DISABLED);
    TP_IP_CON_BIT = 0;
    INTSetVectorPriority(TP_INTERRUPT_VECTOR, TP_INTERRUPT_PRIORITY);
    INTSetVectorSubPriority(TP_INTERRUPT_VECTOR, TP_INTERRUPT_SUBPRIORITY);
    // INT sources are defined in pic32-libs\include\peripheral\int_3xx_4xx.h
    INTClearFlag(INT_INT1);
    INTEnable(INT_INT1, INT_ENABLED);
}

void I2CWait(unsigned int cnt)
{
    while (--cnt)
    {
        asm("nop");
        asm("nop");
    }
}

void TPI2CIdle(void)
{
    UINT8 timeout = 150;
    /* Wait until I2C Bus is Inactive */
    while (TPCONbits.SEN || TPCONbits.PEN || TPCONbits.RCEN ||
            TPCONbits.RSEN || TPCONbits.ACKEN || TPSTATbits.TRSTAT || timeout-- );
}

BOOL TPI2CStart(void)
{
    // wait for module idle
    TPI2CIdle();
    // Enable the Start condition
    TPCONbits.SEN = 1;

    // Check for collisions
    if (TPSTATbits.BCL)
    {
        drvUARTPrintString(_UART_LOG, "I2C Start Cond Error! \r\n");
        drvUARTPrintString(_UART_LOG, "\r\nCON\r\n");
        drvUARTPutHexWord(_UART_LOG, TPI2CCON);

        drvUARTPrintString(_UART_LOG, "\r\nSTAT\r\n");
        drvUARTPutHexWord(_UART_LOG, TPI2CSTAT);
        TPi2cCommError();
        return (FALSE);
    }
    else
    {
        // wait for module idle
        TPI2CIdle();
        return (TRUE);
    }
}

void TPI2CStop(void)
{
    int x = 0;
    // wait for module idle
    TPI2CIdle();
    //initiate stop bit
    TPCONbits.PEN = 1;

    //wait for hardware clear of stop bit
    while (TPCONbits.PEN)
    {
        I2CWait(50);
        x++;
        if (x > 1) break;
    }
    TPCONbits.RCEN = 0;
    // IFS1bits.MI2C1IF = 0; // Clear Interrupt
    TPSTATbits.IWCOL = 0;
    TPSTATbits.BCL = 0;
    // wait for module idle
    TPI2CIdle();
}

BOOL TPI2CSendByte(BYTE data)
{
    //while(CPSTATbits.TBF); //Wait till data is transmitted.
    // Send the byte
    TPI2CTRN = data;

    // Check for collisions
    if ((TPSTATbits.IWCOL == 1))
    {
        drvUARTPrintString(_UART_LOG, "IWOCL \r\n");
        drvUARTPrintString(_UART_LOG, "\r\nCON\r\n");
        drvUARTPutHexWord(_UART_LOG, TPI2CCON);

        drvUARTPrintString(_UART_LOG, "\r\nSTAT\r\n");
        drvUARTPutHexWord(_UART_LOG, TPI2CSTAT);
        TPi2cCommError();
        return (FALSE);
    }
    else
    {
        while (TPSTATbits.TRSTAT); // wait until write cycle is complete
        if ((TPSTATbits.BCL == 1))
        {
            drvUARTPrintString(_UART_LOG, "I2C Tx Error! \r\n");
            drvUARTPrintString(_UART_LOG, "\r\nCON\r\n");
            drvUARTPutHexWord(_UART_LOG, TPI2CCON);

            drvUARTPrintString(_UART_LOG, "\r\nSTAT\r\n");
            drvUARTPutHexWord(_UART_LOG, TPI2CSTAT);
            TPi2cCommError();
            return (FALSE);
        }
        else
        {
            // wait for module idle
            TPI2CIdle();
            return (TRUE);
        }
    }

}

// This function exactly implements the algorithm in the Acp Spec

BOOL iApTPReadRegisterLoop(UINT8 reg, UINT8* rxPtr, UINT8 len)
{

    UINT8 i, flag, ret, j;
    flag = 0;
    ret = FALSE;
#if(iAcp_DEBUG == 1)
    drvUARTPrintString(_UART_LOG, "\r\n-----Start read reg:-----\r\n");
    drvUARTPutHex(_UART_LOG, reg);
    drvUARTPrintString(_UART_LOG, "\r\n");
#endif
    for (i = 0; i < 100; i++) // wait for ACK for some time
    {
        //1. i2c start
        TPI2CStart();
        //TPI2CIdle();
        //2. Set TP in W Mode
        TPI2CSendByte(C_TP_WRITE);
        //3. Check ACK
        TPI2CIdle();
        if (TPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }
        #if(iAcp_DEBUG == 1)
        drvUARTPutChar(_UART_LOG, 'y');
        #endif
    }

    // 4.if write cmd was successful, put the regno on the bus
    if (flag == 1)
    {
        flag = 0;
        TPI2CSendByte(reg);
        TPI2CIdle();
        if (TPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            for (i = 0; i < len; i++) //read all the bytes
            {
                for (j = 0; j < 100; j++)
                {
                    //5.Issue a repeated start = a start cond without a prior stop
                    TPI2CStart();
                    //TPI2CIdle();
                    //6. Set TP in R mode
                    TPI2CSendByte(C_TP_READ);
                    //7. Check ACK
                    if (TPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
                    {
                        flag = 1;
                        break;
                    }
                    #if(iAcp_DEBUG == 1)
                    drvUARTPutChar(_UART_LOG, 'x');
                    #endif
                }
                if (flag == 1)
                {
                    flag = 0;

                    // got the ack, processor is in read mode
                    //8. read the register
                    TPCONbits.RCEN = 1; // enable master read
                    while (TPCONbits.RCEN); // wait for byte to be received !(TPSTATbits.RBF)

                    TPI2CIdle();
                    TPSTATbits.I2COV = 0;
                    *(rxPtr + i) = TPI2CRCV;

                    //9. Generate a NACK
                    TPCONbits.ACKDT = 1; // send nack
                    TPCONbits.ACKEN = 1;

                    //10. generate a stop
                    TPI2CStop();

                    ret = TRUE;
                }
                else
                {
                    //read failed
                    drvUARTPrintString(_UART_LOG, "read failed\r\n");
                    ret = FALSE;
                    TPi2cCommError();
                }
            }
#if(iAcp_DEBUG == 1)
            for (i = 0; i < len; i++)
            {
                drvUARTPutChar(_UART_LOG, '$');
                drvUARTPutHex(_UART_LOG, *(rxPtr + i));
            }
#endif
        }
        else
        {
            drvUARTPrintString(_UART_LOG, "Error NACK Rxed\r\n");
            TPi2cCommError();

        }
    }
    TPI2CStop();
   // drvUARTPrintString(_UART_LOG, "Done\r\n");
    // UART2PrintString("\r\nCON\r\n");
    // UART2PutHexWord(TPI2CCON);
    // UART2PrintString("\r\nSTAT\r\n");
    // UART2PutHexWord(TPI2CSTAT);
    return ret;
}

BOOL TPWriteRegLoop(UINT8 reg, UINT8* data, UINT8 len)
{
    UINT8 i, flag, j;
    flag = 0;
    for (i = 0; i < 100; i++)
    {
        //1. i2c start
        TPI2CStart();
        //2. Set TP in W Mode
        TPI2CSendByte(C_TP_WRITE);
        //3. Check ACK
        TPI2CIdle();
        if (TPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
        {
            flag = 1;
            break;
        }
        #if(iAcp_DEBUG == 1)
        drvUARTPutChar(_UART_LOG, '.');
        #endif
    }
    // 4.if write cmd was successful, put the data on the bus
    if (flag == 1)
    {
        TPI2CSendByte(reg);
        TPI2CIdle();
        for (j = 0; j < len; j++)
        {
            if (TPSTATbits.ACKSTAT == 0) // Did we receive an ACK ?
            {
                TPI2CSendByte(*(data + j));
                // UART2PutDec(*(data + j));
                //  UART2PrintString("ACK\r\n");
            }
            else
            {
                drvUARTPrintString(_UART_LOG, "Error NACK Rxed\r\n");
                TPi2cCommError();
                return FALSE;
            }
        }
        TPI2CStop();
        //  UART2PrintString("\r\nCON\r\n");
        //  UART2PutHexWord(TPI2CCON);
        //  UART2PrintString("\r\nSTAT\r\n");
        //   UART2PutHexWord(TPI2CSTAT);
        //   UART2PrintString("Write over\r\n");
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void TPTest(void)
{
    UINT8 data[128];
    UINT8 i = 0;
    for (i = 0; i < 13; i++)
    {
        // read out registers
        iApTPReadRegisterLoop(i, &data[0], 1);
    }

}



void TPRead(void)
{
    //UINT8 data[128];
    UINT8 temp = 0;
    // Read Gesture ID
    iApTPReadRegisterLoop(C_TP_GESTID, &TpGestureID, 1);
    //Read No of touch points
    iApTPReadRegisterLoop(C_TP_STATUS, &TpNoOfPoints, 1);
    TpNoOfPoints = TpNoOfPoints & C_TP_TOUCH_POINTS_MASK;

    if (TpNoOfPoints >= 1) // If you dont filter this out maybe you can read the event flags meaningfully..
    {
        // Read XL1
        iApTPReadRegisterLoop(C_TP_XL1, &temp, 1);
        TpTouchPoint1.TpX = temp;
        // Read XH1
        iApTPReadRegisterLoop(C_TP_XH1, &temp, 1);
        TpTouchPoint1.TpTouchEventFlag = (temp & C_TP_TOUCH_EVENT_MASK)>>6;
        TpTouchPoint1.TpX = TpTouchPoint1.TpX | ((temp & C_TP_TOUCH_MSB_MASK)<<8);
        // Read YL1
        iApTPReadRegisterLoop(C_TP_YL1, &temp, 1);
        TpTouchPoint1.TpY = temp;
        // Read YH1
        iApTPReadRegisterLoop(C_TP_YH1, &temp, 1);
        TpTouchPoint1.TpTouchID = (temp & C_TP_TOUCH_ID_MASK) >> 4;
        TpTouchPoint1.TpY = TpTouchPoint1.TpY | ((temp & C_TP_TOUCH_MSB_MASK)<<8);

        if (TpNoOfPoints >= 2)
        {
            // Read XL1
            iApTPReadRegisterLoop(C_TP_XL2, &temp, 1);
            TpTouchPoint2.TpX = temp;
            // Read XH1
            iApTPReadRegisterLoop(C_TP_XH2, &temp, 1);
            TpTouchPoint2.TpTouchEventFlag = (temp & C_TP_TOUCH_EVENT_MASK) >>6;
            TpTouchPoint2.TpX = TpTouchPoint2.TpX | ((temp & C_TP_TOUCH_MSB_MASK)<<8);
            // Read YL1
            iApTPReadRegisterLoop(C_TP_YL2, &temp, 1);
            TpTouchPoint2.TpY = temp;
            // Read YH1
            iApTPReadRegisterLoop(C_TP_YH2, &temp, 1);
            TpTouchPoint2.TpTouchID = (temp & C_TP_TOUCH_ID_MASK) >> 4;
            TpTouchPoint2.TpY = TpTouchPoint2.TpY | ((temp & C_TP_TOUCH_MSB_MASK)<<8);
        }
    }
}

void TPi2cCommError(void)
{
    drvUARTPrintString(_UART_LOG, "i2c error, resetting TP \r\n");
    drvInitTP();
}


void __ISR(_EXTERNAL_1_VECTOR, ipl2) _TPEventHandler(void)
{
    INTClearFlag(INT_INT1);                      // clear the interrupt flag
    TPIntFlag = TRUE;
}
