//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HIMAX HX8347 LCD controllers driver
// Author               Date        Comment
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Anton Alkhimenok     05/26/09
 //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef ILI9468L_H

#define ILI9468L_H
#include <plib.h>
#define PMDIN1  PMDIN
#include "GraphicsConfig.h"
#include "GenericTypeDefs.h"
#include "HardwareProfile.h"

/*********************************************************************
* Overview: Additional hardware-accelerated functions can be implemented
*           in the driver. These definitions exclude the PutPixel()-based
*           functions in the primitives layer (Primitive.c file) from compilation.
*********************************************************************/

// Define this to implement Font related functions in the driver.
//#define USE_DRV_FONT
// Define this to implement Line function in the driver.
//#define USE_DRV_LINE
// Define this to implement Circle function in the driver.
//#define USE_DRV_CIRCLE
// Define this to implement FillCircle function in the driver.
//#define USE_DRV_FILLCIRCLE

#define USE_DRV_BAR          // Implement Bar function in the driver.
#define USE_DRV_CLEARDEVICE  // Implement ClearDevice function in the driver.
#define USE_DRV_PUTIMAGE     // Implement PutImage function in the driver.

#ifndef DISP_HOR_RESOLUTION
    #error DISP_HOR_RESOLUTION must be defined in HardwareProfile.h
#endif

#ifndef DISP_VER_RESOLUTION
    #error DISP_VER_RESOLUTION must be defined in HardwareProfile.h
#endif
#ifndef COLOR_DEPTH
    #error COLOR_DEPTH must be defined in HardwareProfile.h
#endif
#ifndef DISP_ORIENTATION
    #error DISP_ORIENTATION must be defined in HardwareProfile.h
#endif


//Overview: Horizontal and vertical screen size.
#if (DISP_HOR_RESOLUTION != 240)
//    #error This driver doesnt supports this resolution. Horisontal resolution must be 240 pixels.
#endif
#if (DISP_VER_RESOLUTION != 320)
  //  #error This driver doesnt supports this resolution. Vertical resolution must be 320 pixels.
#endif


//Overview: Display orientation.
#if (DISP_ORIENTATION != 0) && (DISP_ORIENTATION != 90)
//    #error This driver doesnt support this orientation.
#endif

//Overview: Color depth.
#if (COLOR_DEPTH != 16)
    #error This driver doesnt support this color depth. It should be 16.
#endif


// Clipping region control codes to be used with SetClip(...) function. 
#define CLIP_DISABLE    0   // Disables clipping.
#define CLIP_ENABLE     1   // Enables clipping.

#if 1

//Some basic colors definitions.
#define BLACK           RGB565CONVERT(0, 0, 0)
#define BRIGHTBLUE      RGB565CONVERT(0, 0, 255)
#define BRIGHTGREEN     RGB565CONVERT(0, 255, 0)
#define BRIGHTCYAN      RGB565CONVERT(0, 255, 255)
#define BRIGHTRED       RGB565CONVERT(255, 0, 0)
#define BRIGHTMAGENTA   RGB565CONVERT(255, 0, 255)
#define BRIGHTYELLOW    RGB565CONVERT(255, 255, 0)
#define BLUE            RGB565CONVERT(0, 0, 128)
#define GREEN           RGB565CONVERT(0, 128, 0)
#define CYAN            RGB565CONVERT(0, 128, 128)
#define RED             RGB565CONVERT(128, 0, 0)
#define MAGENTA         RGB565CONVERT(128, 0, 128)
#define BROWN           RGB565CONVERT(255, 128, 0)
#define LIGHTGRAY       RGB565CONVERT(128, 128, 128)
#define DARKGRAY        RGB565CONVERT(64, 64, 64)
#define LIGHTBLUE       RGB565CONVERT(128, 128, 255)
#define LIGHTGREEN      RGB565CONVERT(128, 255, 128)
#define LIGHTCYAN       RGB565CONVERT(128, 255, 255)
#define LIGHTRED        RGB565CONVERT(255, 128, 128)
#define LIGHTMAGENTA    RGB565CONVERT(255, 128, 255)
#define YELLOW          RGB565CONVERT(255, 255, 128)
#define WHITE           RGB565CONVERT(255, 255, 255)
#define GRAY0           RGB565CONVERT(224, 224, 224)
#define GRAY1           RGB565CONVERT(192, 192, 192)
#define GRAY2           RGB565CONVERT(160, 160, 160)
#define GRAY3           RGB565CONVERT(128, 128, 128)
#define GRAY4           RGB565CONVERT(96, 96, 96)
#define GRAY5           RGB565CONVERT(64, 64, 64)
#define GRAY6           RGB565CONVERT(32, 32, 32)

#endif
extern WORD     _color;
extern SHORT    _clipRgn;
extern SHORT    _clipLeft;
extern SHORT    _clipTop;
extern SHORT    _clipRight;
extern SHORT    _clipBottom;

// Delays execution for PMP cycle time.
#define PMPWaitBusy()   while(PMMODEbits.BUSY);

//Writes index register.
#define SetIndex(index)     \
    RS_LAT_BIT = 0;         \
    PMDIN1 = index;         \
    PMPWaitBusy();

//Writes command.
    #define WriteParameter(cmd) \
    RS_LAT_BIT = 1;           \
    PMDIN1 = cmd;             \
    PMPWaitBusy();

//Writes data
    #define WriteData(data)     \
    RS_LAT_BIT = 1;             \
    PMDIN1 = data;              \
    PMPWaitBusy();



//Writes address pointer.x - horizontal position, y - vertical position
#define SetAddress(x, y)     \
	SetIndex(0x2A);\
	WriteParameter(((WORD_VAL) (WORD) x).v[1]);\
	WriteParameter(((WORD_VAL) (WORD) x).v[0]);\
	WriteParameter(1);		\
	WriteParameter(0xDF);	\
	SetIndex(0x2B);\
	WriteParameter(((WORD_VAL) (WORD) y).v[1]); \
	WriteParameter(((WORD_VAL) (WORD) y).v[0]);\
	WriteParameter(1);			\
	WriteParameter(0x3F);		\
	SetIndex(0x2C); \         

/*\
SetIndex(0x02);                           \
WriteCommand(((WORD_VAL) (WORD) x).v[1]); \
SetIndex(0x03);                           \
WriteCommand(((WORD_VAL) (WORD) x).v[0]); \
SetIndex(0x06);                           \
WriteCommand(((WORD_VAL) (WORD) y).v[1]); \
SetIndex(0x07);                           \
WriteCommand(((WORD_VAL) (WORD) y).v[0]); \
SetIndex(0x22);
*/
//Initializes LCD module.
void    ResetDevice(void);

#if (DISP_ORIENTATION == 90)
    #define GetMaxX()   (DISP_VER_RESOLUTION - 1)
#elif (DISP_ORIENTATION == 0)
    #define GetMaxX()   (DISP_HOR_RESOLUTION - 1)
#endif

#if (DISP_ORIENTATION == 90)
    #define GetMaxY()   (DISP_HOR_RESOLUTION - 1)
#elif (DISP_ORIENTATION == 0)
    #define GetMaxY()   (DISP_VER_RESOLUTION - 1)
#endif

// Input: color - Color coded in 5:6:5 RGB format.

#define SetColor(color) _color = color;
#define GetColor()  _color

#define SetActivePage(page)
#define SetVisualPage(page)

void    PutPixel(SHORT x, SHORT y);
WORD    GetPixel(SHORT x, SHORT y);

#define  SetClipRgn(left, top, right, bottom)        \
        _clipLeft = left;                            \
        _clipTop = top;                              \
        _clipRight = right;                          \
        _clipBottom = bottom;


#define GetClipLeft()       _clipLeft
#define GetClipRight()      _clipRight
#define GetClipTop()        _clipTop
#define GetClipBottom()     _clipBottom
#define SetClip(control)    _clipRgn = control;
//#define IsDeviceBusy()      0
#define SetPalette(colorNum, color)
void    DelayMs(WORD time);
UINT16 LCDReadRegister(UINT8 reg);
void TestILITek(void);
void ClearILITek(UINT16 color);
#endif
