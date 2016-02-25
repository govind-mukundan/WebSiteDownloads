
#ifndef _HARDWARE_PROFILE_H_
#define _HARDWARE_PROFILE_H_

// A simplified version of HardwareProfile.h containing only those definitions used by i2c drivers [PIC24/32]

#if defined(__dsPIC33F__) || defined(__PIC24F__) || defined(__PIC24FK__) || defined(__PIC24H__)
#include <p24fxxxx.h>

#define GetSystemClock()                   32000000UL       // define the system clock appropriately!
#define GetPeripheralClock()              (GetSystemClock() / 2) // On Pic24 peripheral clock is sys clk / 2

#elif defined(__PIC32MX__)

#include <p32xxxx.h>
#include <plib.h>

#define GetSystemClock()        (80000000ul)
#define GetPeripheralClock()    (GetSystemClock() / (1 << OSCCONbits.PBDIV))
#define GetInstructionClock()   (GetSystemClock())

#endif

#endif
