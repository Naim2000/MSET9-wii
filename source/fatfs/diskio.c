/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

#include <sdcard/wiisd_io.h>

/* Definitions of physical drive number for each drive */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv) {
	DSTATUS stat = 0;

	if (pdrv > 0) return STA_NOINIT;

	if (!__io_wiisd.isInserted()) stat |= STA_NODISK;

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(BYTE pdrv) {
	DSTATUS stat = 0;

	if (pdrv > 0) return STA_NOINIT;

	if (!__io_wiisd.startup()) stat |= STA_NOINIT;
	else if (!__io_wiisd.isInserted()) stat |= STA_NODISK;

	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
	if (pdrv > 0) return RES_PARERR;

	return __io_wiisd.readSectors(sector, count, buff) ? RES_OK : RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
	if (pdrv > 0) return RES_PARERR;

	return __io_wiisd.writeSectors(sector, count, buff) ? RES_OK : RES_ERROR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
	if (pdrv > 0) return RES_PARERR;

	return RES_OK;
}

