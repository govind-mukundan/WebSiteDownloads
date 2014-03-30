
/*****************************************************************************
 *  Wrapper APIs to interface FATFs diskio routines with Microchip USB Mass Storage Library code
 *  Once you integrate this file and diskio.c/h to your project, you can do away with SD-SPI.c/h
 *  Please note that this is only for USB Mass Storage Device NOT Host
 *
 *  Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Govind Mukundan      02/15/2012    First version tested with MAL v2011-07-14 on custom PIC24FJ256GB106 board
 *****************************************************************************/

#ifndef _USBDEVFATFS_H
#define _USBDEVFATFS_H
#include "FSDefs.h" // MEDIA_INFORMATION



MEDIA_INFORMATION * ffs_DiskInitialize(void);
BYTE ffs_MediaDetect(void);
BYTE ffs_SectorRead(DWORD sector_addr, BYTE* buffer);
BYTE ffs_SectorWrite(DWORD sector_addr, BYTE* buffer, BYTE allowWriteToZero);
BYTE ffs_WriteProtectState(void);
WORD ffs_ReadSectorSize(void);
DWORD ffs_ReadCapacity(void);
void ffs_InitIO(void);
void ffs_ShutDownMedia(void);

#endif
