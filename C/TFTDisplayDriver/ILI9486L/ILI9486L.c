/*****************************************************************************
 *  HIMAX HX8347 controller driver
 *  Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Govind Mukundan      Oct/06/2012    Added support for ILI9486L (tested with 16-bit bus)
 *****************************************************************************/
#include "Graphics\Graphics.h"
#include "ILI9486L.h"

// Color
WORD _color;
SHORT _clipRgn;
SHORT _clipLeft;
SHORT _clipTop;
SHORT _clipRight;
SHORT _clipBottom;

void SetReg(BYTE index, BYTE value);
void SetReg3Param(BYTE index, BYTE value1, BYTE value2, BYTE value3);
void SetReg2Param(BYTE index, BYTE value1, BYTE value2);
void SetReg0Param(BYTE index);
extern WORD IsDeviceBusy(void);

WORD IsDeviceBusy(void)
{

return 0;

}

void WaitForVsync(void)
{

// This function blocks until the LCD panel has stopped updatng diaplay fron RAM !! Comment it out if you dont have the TE signal!!
// You can transmit a new frame buffer/elements at this point.
// wait for TE line to go high to low
	if(TEAR_STATUS_BIT == 1)
{
	while(TEAR_STATUS_BIT == 1) {;}
}
else
{
	while(TEAR_STATUS_BIT == 0) {;}
	while(TEAR_STATUS_BIT == 1) {;}
}

}
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
    WriteParameter(value);
    CS_LAT_BIT = 1;
}

void SetReg0Param(BYTE index)
{
    CS_LAT_BIT = 0;
    SetIndex(index);
    CS_LAT_BIT = 1;
}

void SetReg3Param(BYTE index, BYTE value1, BYTE value2, BYTE value3)
{
    CS_LAT_BIT = 0;
    SetIndex(index);
   WriteParameter(value1);
	WriteParameter(value2);
	WriteParameter(value3);
    CS_LAT_BIT = 1;
}

void SetReg2Param(BYTE index, BYTE value1, BYTE value2)
{
    CS_LAT_BIT = 0;
    SetIndex(index);
    WriteParameter(value1);
	WriteParameter(value2);
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
TEAR_STATUS_TRIS = 1;
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
    PMMODEbits.MODE = 2; // Master 2 -> separate WR and RD strobe lines
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
    //PMMODEbits.MODE16 = 0; // 8 bit mode
#endif

    PMCONbits.CSF = 0;
    PMCONbits.PTRDEN = 1;
    PMCONbits.PTWREN = 1;
    PMCONbits.PMPEN = 1;

    // Release from reset
	RST_LAT_BIT = 1;
    DelayMs(100);
    RST_LAT_BIT = 0;
    DelayMs(100);
	RST_LAT_BIT = 1;
	DelayMs(100);

	SetReg(0x36, 0x68); // memory access control MY=0, MX=1, MV=1, BGR=1
	SetReg(0x3A,0x05);  //Interface Pixel Format  ( DPI[3:0] - DBI[2:0] )    16 bit/pixel, CPU interface format
// DM, RM = 0
	SetReg(0xB0,0x00);  //Interface Mode Control (SDA_EN 000 VSPL HSPL DPL EPL )ARM£­£­DPL: PCLK polarity set (¡°0¡±=data fetched at the rising time, //¡°1¡±=data fetched at the falling time)SET DPL=1
	SetReg3Param(0xB6,0x00,0x42,0x3B); //Display Function Control
	SetReg(0xB4,0x00); //(000 ZINV 00 DINV[1:0])	Z-INV  DINV 
SetReg2Param(0xB1,0x80,0x11); // frame rate = 50Hz, Horizontal line period = 17clk (what does this mean??)
SetReg(0xB7, 0x07); // extra

	// Power settings
	SetReg2Param(0xC1,0x41,0x00); //(0100 0 BT[2:0]) (0000 0 VC[2:0])
	SetReg3Param(0xC5,0x00,0x53,0x00);
	SetReg(0xC2, 0x33); //extra
	SetReg(0x35, 0x00); // enable tearing effect line
	//Gamma Setting_10323
	CS_LAT_BIT = 0;
    SetIndex(0xE0);
    WriteParameter(0x0F);
	WriteParameter(0x1B);
	WriteParameter(0x18);
	WriteParameter(0x0B);
	WriteParameter(0x0E);
	WriteParameter(0x09);
	WriteParameter(0x47);
	WriteParameter(0x94);
	WriteParameter(0x35);
	WriteParameter(0x0A);
	WriteParameter(0x13);
	WriteParameter(0x05);
	WriteParameter(0x08);
	WriteParameter(0x03);
	WriteParameter(0x00);
	CS_LAT_BIT = 1;

		//Gamma Setting_10323
	CS_LAT_BIT = 0;
    SetIndex(0xE1);
    WriteParameter(0x0F);
	WriteParameter(0x3A);
	WriteParameter(0x37);
	WriteParameter(0x0B);
	
	WriteParameter(0x0C);
	WriteParameter(0x05);
	WriteParameter(0x4A);
	WriteParameter(0x24);
	
	WriteParameter(0x39);
	WriteParameter(0x07);
	WriteParameter(0x10);
	WriteParameter(0x04);
	
	WriteParameter(0x27);
	WriteParameter(0x25);
	WriteParameter(0x00);
	CS_LAT_BIT = 1;
	
	CS_LAT_BIT = 0;
	SetIndex(0x11);
	CS_LAT_BIT = 1;
	DelayMs(100);
	CS_LAT_BIT = 0;
	SetIndex(0x29);
	CS_LAT_BIT = 1;
	
}

/*********************************************************************
 * Function: void PutPixel(SHORT x, SHORT y)
 * Input: x,y - pixel coordinates
 * Overview: puts pixel
 WriteCommand(((WORD_VAL) (WORD) x).v[1]); \
SetIndex(0x03);                           \
WriteCommand(((WORD_VAL) (WORD) x).v[0]); \
SetIndex(0x06);                           \
WriteCommand(((WORD_VAL) (WORD) y).v[1]); \
SetIndex(0x07);                           \
WriteCommand(((WORD_VAL) (WORD) y).v[0]); \
SetIndex(0x22);
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
// modify the StartPage and StartColumn values
    CS_LAT_BIT = 0; // select device
	SetIndex(0x2A);
	// send data
	WriteParameter(((WORD_VAL) (WORD) x).v[1]);
	WriteParameter(((WORD_VAL) (WORD) x).v[0]);
	WriteParameter(1);
	WriteParameter(0xDF);	
	SetIndex(0x2B);
	// send data
	WriteParameter(((WORD_VAL) (WORD) y).v[1]); 
	WriteParameter(((WORD_VAL) (WORD) y).v[0]);
	WriteParameter(1);
	WriteParameter(0x3F);
	// Now update memory
	SetIndex(0x2C); // command for memory
	//IsDeviceBusy(); // wait for device to be ready
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
WORD BarILITek(SHORT left, SHORT top, SHORT right, SHORT bottom)
{
    register UINT16 x;

    if (_clipRgn)
    {
        if (left < _clipLeft) left = _clipLeft;
        if (right > _clipRight) right = _clipRight;
        if (top < _clipTop) top = _clipTop;
        if (bottom > _clipBottom) bottom = _clipBottom;
    }

    CS_LAT_BIT = 0;
	SetIndex(0x2A); // Start column and end column
	// send data
	WriteParameter(((WORD_VAL) (WORD) left).v[1]);
	WriteParameter(((WORD_VAL) (WORD) left).v[0]);
	WriteParameter(((WORD_VAL) (WORD) right).v[1]);
	WriteParameter(((WORD_VAL) (WORD) right).v[0]);
	SetIndex(0x2B);
	// send data
	WriteParameter(((WORD_VAL) (WORD) top).v[1]);
	WriteParameter(((WORD_VAL) (WORD) top).v[0]);
	WriteParameter(((WORD_VAL) (WORD) bottom).v[1]);
	WriteParameter(((WORD_VAL) (WORD) bottom).v[0]);

SetIndex(0x2C); // command for memory
for (x = 0; x < (DWORD) (bottom-top + 1)*(right-left + 1); x++)
    {
        WriteData(_color);
    }
    CS_LAT_BIT = 1;
    return 1;
}

WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom)
{
    register UINT16 x;

    if (_clipRgn)
    {
        if (left < _clipLeft) left = _clipLeft;
        if (right > _clipRight) right = _clipRight;
        if (top < _clipTop) top = _clipTop;
        if (bottom > _clipBottom) bottom = _clipBottom;
    }

    CS_LAT_BIT = 0;
	SetIndex(0x2A); // Start column and end column
	// send data
	WriteParameter(((WORD_VAL) (WORD) left).v[1]);
	WriteParameter(((WORD_VAL) (WORD) left).v[0]);
	WriteParameter(((WORD_VAL) (WORD) right).v[1]);
	WriteParameter(((WORD_VAL) (WORD) right).v[0]);
	SetIndex(0x2B);
	// send data
	WriteParameter(((WORD_VAL) (WORD) top).v[1]);
	WriteParameter(((WORD_VAL) (WORD) top).v[0]);
	WriteParameter(((WORD_VAL) (WORD) bottom).v[1]);
	WriteParameter(((WORD_VAL) (WORD) bottom).v[0]);
	SetIndex(0x2C); // command for memory
	for (x = 0; x < (DWORD) (bottom-top + 1)*(right-left + 1); x++)
    {
        WriteData(_color);
    }
    return 1;
}

// Overview: clears screen with current color

void ClearDevice(void)
{
ClearILITek(_color);

}

void ClearILITek(UINT16 color)
{
// modify the StartPage and StartColumn values
    CS_LAT_BIT = 0; // select device
	// send command to modify sc and sp registers
	SetIndex(0x2A);
	// send data
	WriteParameter(0);
	WriteParameter(0);
	WriteParameter(1);
	WriteParameter(0xDF);

	SetIndex(0x2B);
	// send data
	WriteParameter(0);
	WriteParameter(0);
	WriteParameter(1);
	WriteParameter(0x3F);
	// Now update memory
	SetIndex(0x2C); // command for memory
    DWORD counter;
	WaitForVsync();
    for (counter = 0; counter < (DWORD) (320 + 1)*(480 + 1); counter++)
    {
        WriteData(color);
    }
    CS_LAT_BIT = 1;
}

UINT16 LCDReadRegister(UINT8 reg)
{
    UINT16 ret = 0;
	UINT16 ret2 = 0;
    // Write command to read the register, which is the reg value
    CS_LAT_BIT = 0;
    RS_LAT_BIT = 0;
    PMDIN1 = reg;
    PMPWaitBusy();
    // Read from register pointed to
	// First value read is the last value on the bus, so it should be discarded, the ral value is obtained on the 2nd read
    CS_LAT_BIT = 0;
    RS_LAT_BIT = 1;
    ret = PMDIN1;
    PMPWaitBusy();
    
    my_printf("dummy = %d\r\n", ret);
    ret = PMDIN1;
    PMPWaitBusy();
	ret = PMDIN1;
    PMPWaitBusy();
	my_printf("%x = (%x, %x)\r\n", reg, ret, ret2);
    CS_LAT_BIT = 1;

    return ret;
}

BOOL ILITekReadRegister(UINT8 reg, UINT8 noOfParam)
// no of param is the number of parameters excluding the dummy read
{
    UINT16 ret = 0;
	UINT8 i = 0;
    // Write command to read the register, which is the reg value
    CS_LAT_BIT = 0;
    RS_LAT_BIT = 0;
    PMDIN1 = reg;
    PMPWaitBusy();
    // Read from register pointed to
	// First value read is the last value on the bus, so it should be discarded, the ral value is obtained on the 2nd read
    CS_LAT_BIT = 0;
    RS_LAT_BIT = 1;
    ret = PMDIN1;
    PMPWaitBusy(); // dummy read x1
    ret = PMDIN1;
    PMPWaitBusy(); // dummy read x2
    my_printf("(reg,dummy_rd) = (%x,%x)\r\n", reg,ret);
	my_printf("-----------\r\n");
	for(i = 0; i<noOfParam; i++)
	{
    ret= PMDIN1;
    PMPWaitBusy(); // real read
	my_printf("$%x", ret);
	}
	my_printf("\r\n");
	my_printf("-----------\r\n");
    CS_LAT_BIT = 1;

    return TRUE;
}

void TestILITek(void)
{

ILITekReadRegister(0x04,3);// Read Display ID
ILITekReadRegister(0x05,1); // Errors on DSI
ILITekReadRegister(0x09,4); // Errors Disp Satus
ILITekReadRegister(0x0A,1); // Power Mode
ILITekReadRegister(0x0B,1); // MADCTL
ILITekReadRegister(0x0C,1); // Read Pixel
ILITekReadRegister(0x0D,1); // Read Image Mode
ILITekReadRegister(0x0E,1); // Read Signal Mode
ILITekReadRegister(0x0F,1); // Read SElf Diag
ILITekReadRegister(0x2E,3); // Read Memory
ILITekReadRegister(0x52,1); // Read Disp Brightness
ILITekReadRegister(0xDA,1); // Read ID1
ILITekReadRegister(0xDB,1); // Read ID2
ILITekReadRegister(0xDC,1); // Read ID3
}
