/*****************************************************************************
 *  Wrapper APIs to interface FATFs diskio routines with Microchip USB Mass Storage Library code
 *  Once you integrate this file and diskio.c/h to your project, you can do away with SD-SPI.c/h
 *  Please note that this is for USB Mass Storage Device NOT Host
 * 
 *  Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Govind Mukundan      02/15/2012    First version tested with MAL v2011-07-14 on custom PIC24FJ256GB106 board
 *****************************************************************************/


#include "GenericTypeDefs.h"
#include "Compiler.h"
#include "usbdevFatFs.h"
#include "HardwareProfile.h"
#include "USB/usb.h"
#include "diskio.h"
#include "./USB/usb_function_msd.h"


#if defined(__C30__) || defined(__C32__)
LUN_FUNCTIONS LUN[MAX_LUN + 1] = {
    {
        &ffs_DiskInitialize,
        &ffs_ReadCapacity,
        &ffs_ReadSectorSize,
        &ffs_MediaDetect,
        &ffs_SectorRead,
        &ffs_WriteProtectState,
        &ffs_SectorWrite
    }
};
#endif

//#define addToTerminalLog(x)

static MEDIA_INFORMATION mediaInformationffs;

MEDIA_INFORMATION * ffs_DiskInitialize(void)
{
    DSTATUS status;
    WORD sector_size;
    DWORD sector_count;
    status = disk_initialize(0); // interpret the result
    mediaInformationffs.errorCode = MEDIA_CANNOT_INITIALIZE;
    mediaInformationffs.validityFlags.value = 0;
    mediaInformationffs.validityFlags.bits.sectorSize = 0;
    mediaInformationffs.maxLUN = 0;
    if ((status & (STA_NODISK | STA_NOINIT)) == 0)
    {
        mediaInformationffs.errorCode = MEDIA_NO_ERROR;
        if (disk_ioctl(0,GET_SECTOR_SIZE, &sector_size) == RES_OK)
        {
            mediaInformationffs.validityFlags.bits.sectorSize = 1;
            mediaInformationffs.sectorSize = sector_size;
        }
        if (disk_ioctl(0,GET_SECTOR_COUNT, &sector_count) == RES_OK)
        {
            mediaInformationffs.validityFlags.bits.maxLUN = 1;
            mediaInformationffs.maxLUN = sector_count - 1;
        }
    }
    addToTerminalLog("Disk Init");
    return &mediaInformationffs;


}

BYTE ffs_MediaDetect(void)
{
    
    BYTE result = FALSE;
    
    if((STA_NODISK & (disk_status(0))) == 0)
        result = TRUE;
    
return result;
}


 BYTE ffs_SectorRead(DWORD sector_addr, BYTE* buffer)
{
     BYTE result = FALSE;

    // Assuming sectors = 512B, LBA = logical block address, which is just the index of the sector, MAL passes the same thing
    // and converts the sector number to address in MDD_SDSPI_AsyncReadTasks
if(RES_OK == disk_read(0, buffer, sector_addr, 1 ))
{
    result = TRUE;
         addToTerminalLog("SR OK");
}
else
{
    addToTerminalLog("SR NOK");
}

return(result);

}


BYTE ffs_SectorWrite(DWORD sector_addr, BYTE* buffer, BYTE allowWriteToZero)
{
     BYTE result = FALSE;
    // Assuming sectors = 512B, LBA = logical block address, which is just the index of the sector, MAL passes the same thing
    // and converts the sector number to address in MDD_SDSPI_AsyncReadTasks
     // No restriction on writing to MBR (LBA = 0)
if(RES_OK == disk_write(0, buffer, sector_addr, 1 ))
{
    result = TRUE;
    addToTerminalLog("SW OK");
}
else
{
    addToTerminalLog("SW NOK");
}
return(result);

}



BYTE ffs_WriteProtectState(void)
{
return (BYTE)(STA_PROTECT & disk_status(0));
}

WORD ffs_ReadSectorSize(void)
{
    return(512); // hard code

}

DWORD ffs_ReadCapacity(void)
{
    DWORD cnt = 0;
    if (RES_OK == disk_ioctl( 0, GET_SECTOR_COUNT, &cnt ))
    {
        addToTerminalLog("RC OK");
        return (cnt - 1);

    }
    else
    {
        addToTerminalLog("RC NOK");
        return 0;
    }

}

void ffs_InitIO(void)
{
// Not used by usb_function_msd.c
}

void ffs_ShutDownMedia(void)
{
// Not used by usb_function_msd.c
}
