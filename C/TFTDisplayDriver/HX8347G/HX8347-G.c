/*****************************************************************************
 *  HIMAX HX8347 controller driver
 *  Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Govind Mukundan      01/05/2012    Added support for HX8347-G model (tested with 16-bit bus)
 *****************************************************************************/
#include "Graphics\Graphics.h"

// Color
WORD _color;
SHORT _clipRgn;
SHORT _clipLeft;
SHORT _clipTop;
SHORT _clipRight;
SHORT _clipBottom;

void SetReg(BYTE index, BYTE value);

#if 0

void DelayMs(WORD time)
{
    while (time--)
    {
        unsigned int int_status;
        int_status = INTDisableInterrupts();
        OpenCoreTimer(GetSystemClock() / 2000); // core timer is at 1/2 system clock
        INTRestoreInterrupts(int_status);
        mCTClearIntFlag();
        while (!mCTGetIntFlag());
    }

    mCTClearIntFlag();
}
#endif

// Input: index - register number,value - value to be set
// Overview: sets graphics controller register

void SetReg(BYTE index, BYTE value)
{
    CS_LAT_BIT = 0;
    SetIndex(index);
    WriteCommand(value);
    CS_LAT_BIT = 1;
}

int GetReg(BYTE index)
{
    int x;
    CS_LAT_BIT = 0;
    SetIndex(index);
    DelayMs(1);
    RS_LAT_BIT = 1;
    x = PMDIN; // start a read sequence
    PMPWaitBusy(); // wait for read completion
    x = PMDIN;
    CS_LAT_BIT = 1;
    return x;
}

// Overview: resets LCD, initializes PMP

void ResetDevice(void)
{
    int i;

    // Hold in reset
    RST_LAT_BIT = 0;
    // Set reset pin as output
    RST_TRIS_BIT = 0;

    // Enable data access
    RS_LAT_BIT = 1;
    // Set RS pin as output
    RS_TRIS_BIT = 0;

    // Disable LCD 
    CS_LAT_BIT = 1;
    // Set LCD CS pin as output
    CS_TRIS_BIT = 0;

    // PMP setup 
    PMMODE = 0;
    PMAEN = 0;
    PMCON = 0;
    PMMODEbits.MODE = 2; // Master 2
    PMMODEbits.WAITB = 0;
#ifdef __PIC32MX
    PMMODEbits.WAITM = 3;
#else
    PMMODEbits.WAITM = 2;
#endif
    PMMODEbits.WAITE = 0;

#ifdef USE_16BIT_PMP
    PMMODEbits.MODE16 = 1; // 16 bit mode
#else
    PMMODEbits.MODE16 = 0; // 8 bit mode
#endif

    PMCONbits.CSF = 0;
    PMCONbits.PTRDEN = 1;
    PMCONbits.PTWREN = 1;
    PMCONbits.PMPEN = 1;

    // Release from reset
    DelayMs(100);
    RST_LAT_BIT = 1;
    DelayMs(150);

    // Driving ability setting
    SetReg(0x2E, 0x89);  //GDOFF
    SetReg(0x29, 0X8F); //RTN
    SetReg(0x2B, 0X02);  //DUM
    SetReg(0xE2, 0X00);  //VREF
    SetReg(0xE4, 0X01);  //EQ
    SetReg(0xE5, 0X10);  //EQ
    SetReg(0xE6, 0X01);  //EQ
    SetReg(0xE7, 0X10);  //EQ
    SetReg(0xE8, 0X70);  //OPON
    SetReg(0xF2, 0X00);  //GEN
    SetReg(0xEA, 0X00);  //PTBA
    SetReg(0xEB, 0X20);  //PTBA
    SetReg(0xEC, 0X3C);  //STBA
    SetReg(0xED, 0XC8);  //STBA
    SetReg(0xE9, 0X38);  //OPON1
    SetReg(0xF1, 0X01);  //OTPS1B

    // Gamma 2.8 setting
    SetReg(0x40, 0X00);  
    SetReg(0x41, 0X00);  
    SetReg(0x42, 0X00);  
    SetReg(0x43, 0X15);  
    SetReg(0x44, 0X13);  
    SetReg(0x45, 0X3f);  
    SetReg(0x47, 0X55);  
    SetReg(0x48, 0X00);  
    SetReg(0x49, 0X12);  
    SetReg(0x4A, 0X19);  
    SetReg(0x4B, 0X19);  
    SetReg(0x4C, 0X16);  
    SetReg(0x50, 0X00);  
    SetReg(0x51, 0X2c);  
    SetReg(0x52, 0X2a);  
    SetReg(0x53, 0X3F);  
    SetReg(0x54, 0X3F);  
    SetReg(0x55, 0X3F);  
    SetReg(0x56, 0X2a);  
    SetReg(0x57, 0X7e);  
    SetReg(0x58, 0X09);  
    SetReg(0x59, 0X06);  
    SetReg(0x5A, 0X06);  
    SetReg(0x5B, 0X0d);  
    SetReg(0x5C, 0X1F);  
    SetReg(0x5D, 0XFF);  

    // Power Voltage Setting
    SetReg(0x1B, 0X1A); 
    SetReg(0x1A, 0X02); 
    SetReg(0x24, 0X61); 
    SetReg(0x25, 0X5C); 

    // Vcom offset
    //   SetReg(0x23,0x8D);   // FLICKER ADJUST
    SetReg(0x23, 0x62); 

    // Power ON Setting
    SetReg(0x18, 0X36); //RADJ 70Hz
    SetReg(0x19, 0X01); //OSC_EN=1
    SetReg(0x1F, 0X88); // GAS=1, VOMG=00, PON=0, DK=1, XDK=0, DVDH_TRI=0, STB=0
    DelayMs(5);
    SetReg(0x1F, 0X80); // GAS=1, VOMG=00, PON=0, DK=0, XDK=0, DVDH_TRI=0, STB=0
    DelayMs(5);
    SetReg(0x1F, 0X90); // GAS=1, VOMG=00, PON=1, DK=0, XDK=0, DVDH_TRI=0, STB=0
    DelayMs(5);
    SetReg(0x1F, 0XD4); // GAS=1, VOMG=10, PON=1, DK=0, XDK=1, DDVDH_TRI=0, STB=0
    DelayMs(5);
    //262k/65k color selection
    SetReg(0x17, 0X05);  //default 0x06 262k color // 0x05 65k color

    //SET PANEL
    SetReg(0x36, 0X09);  //SS_P, GS_P,REV_P,BGR_P

    //Display ON Setting
    SetReg(0x28, 0X38);  //GON=1, DTE=1, D=1000
    DelayMs(40);
    SetReg(0x28, 0X3C);  //GON=1, DTE=1, D=1100
    
    // 0x13F = 320, 0xEF = 240
    // The origin of the display can be adjusted to any of the 4 corners by adjusting these registers
#if (DISP_ORIENTATION == 0)
    //Set GRAM Area
    SetReg(0x02, 0X00); 
    SetReg(0x03, 0X00);  //Column Start
    SetReg(0x04, 0X00); 
    SetReg(0x05, 0XEF);  //Column End
    SetReg(0x06, 0X00); 
    SetReg(0x07, 0X00);  //Row Start
    SetReg(0x08, 0X01); 
    SetReg(0x09, 0X3F);  //Row End
    SetReg(0x16,0x08);   // MY=0, MX=0, MV=0, BGR=1
#else // orientation is 90
    SetReg(0x02, 0X00); 
    SetReg(0x03, 0X00);  //Column Start
    SetReg(0x04, 0X01); 
    SetReg(0x05, 0X3F);  //Column End
    SetReg(0x06, 0X00); 
    SetReg(0x07, 0X00);  //Row Start
    SetReg(0x08, 0X00); 
    SetReg(0x09, 0XEF);  //Row End
    SetReg(0x16,0x60);   // MY=0, MX=1, MV=1, BGR=1
#endif
}

/*********************************************************************
 * Function: void PutPixel(SHORT x, SHORT y)
 * Input: x,y - pixel coordinates
 * Overview: puts pixel
 ********************************************************************/
void PutPixel(SHORT x, SHORT y)
{
    if (_clipRgn)
    {
        if (x < _clipLeft) return;
        if (x > _clipRight) return;
        if (y < _clipTop) return;
        if (y > _clipBottom) return;
    }

    CS_LAT_BIT = 0;
    SetAddress(x, y);
    WriteData(_color);
    CS_LAT_BIT = 1;
}

/*********************************************************************
 * Function: WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom)
 * Input: left,top - top left corner coordinates,
 *        right,bottom - bottom right corner coordinates
 * Side Effects: none
 * Overview: draws rectangle filled with current color
 ********************************************************************/
WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom)
{
    register SHORT x, y;

#ifndef USE_NONBLOCKING_CONFIG
    while (IsDeviceBusy() != 0); /* Ready */
#else
    if (IsDeviceBusy() != 0) return 0;
#endif

    if (_clipRgn)
    {
        if (left < _clipLeft) left = _clipLeft;
        if (right > _clipRight) right = _clipRight;
        if (top < _clipTop) top = _clipTop;
        if (bottom > _clipBottom) bottom = _clipBottom;
    }

    CS_LAT_BIT = 0;
    for (y = top; y < bottom + 1; y++)
    {
        SetAddress(left, y);
        for (x = left; x < right + 1; x++)
        {
            WriteData(_color);
        }
    }
    CS_LAT_BIT = 1;
    return 1;
}

// Overview: clears screen with current color

void ClearDevice(void)
{
    DWORD counter;
    CS_LAT_BIT = 0;
    SetAddress(0, 0);
    for (counter = 0; counter < (DWORD) (GetMaxX() + 1)*(GetMaxY() + 1); counter++)
    {
        WriteData(_color);
    }
    CS_LAT_BIT = 1;
}

UINT16 LCDReadRegister(UINT8 reg)
{
    UINT16 ret = 0;
    // Write 00h to index register
    CS_LAT_BIT = 0;
    RS_LAT_BIT = 0;
    PMDIN1 = reg;
    PMPWaitBusy();
    // Read from register pointed to by index register
    CS_LAT_BIT = 0;
    RS_LAT_BIT = 1;
    ret = PMDIN1;
    PMPWaitBusy();
    
    my_printf("dummy = %d\r\n", ret);
    ret = PMDIN1;
    PMPWaitBusy();
    CS_LAT_BIT = 1;

    my_printf("R%x = %x", reg, ret);

    return ret;
}
