

#ifndef DRVTP_H
#define DRVTP_H


/*

Event Flag(the 7th/6th bit of  TOUCH_XH ):  the  action of finger
00b Put Down (first time finger touch)
01b Put Up
10b Contact

Touch ID(the 7th/6th/5th/4th bit of TOUCH_YH )?the ID of finger
0 or  1


 */

#define TP_INT_LAT      _RE8
#define TP_INT_TRIS     _TRISE8

#define TP_RESET_LAT    _LATD8
#define TP_RESET_TRIS   _TRISD8
#define TP_IP_CON_BIT   INTCONbits.INT1EP

#define TP_INTERRUPT_PRIORITY       INT_PRIORITY_LEVEL_2
#define TP_INTERRUPT_SUBPRIORITY    INT_SUB_PRIORITY_LEVEL_0
// INT vectors are defined in pic32mx....h device file
#define TP_INTERRUPT_VECTOR         INT_EXTERNAL_1_VECTOR

// CP uses i2c 1, address is 0x3E
#define TPI2CCON    I2C1CON
#define TPCONbits   I2C1CONbits
#define TPI2CSTAT   I2C1STAT
#define TPSTATbits  I2C1STATbits
#define TPI2CMSK     I2C1MSK
#define TPI2CRCV     I2C1RCV
#define TPI2CTRN     I2C1TRN
#define TPI2CADD     I2C1ADD
#define TPI2CBRG    I2C1BRG

#define C_TP_ADDR   0x3E
#define C_TP_READ   0x7D
#define C_TP_WRITE  0x7C

#define C_TP_GESTID 0x01
#define C_TP_STATUS 0x02
#define C_TP_XH1    0x03
#define C_TP_XL1    0x04
#define C_TP_YH1    0x05
#define C_TP_YL1    0x06
#define C_TP_XH2    0x09
#define C_TP_XL2    0x0A
#define C_TP_YH2    0x0B
#define C_TP_YL2    0x0C

#define C_TP_TOUCH_POINTS_MASK  0x0F
#define C_TP_TOUCH_EVENT_MASK   0xC0
#define C_TP_TOUCH_MSB_MASK     0x0F
#define C_TP_TOUCH_ID_MASK      0xF0


// calculate baud rate of I2C
#define Fosc       (GetSystemClock())
#define Fcy        (Fosc/2)
#define Fsck       100000
#define I2C_BRG    ((GetPeripheralClock()/(2*Fsck))-1)

typedef struct {
    UINT16 TpX;
    UINT16 TpY;
    UINT8 TpTouchID;
    UINT8 TpTouchEventFlag;
}ts_TpDataPoint;


void drvInitTP(void);
void TPTest(void);
void TPRead(void);

#endif
