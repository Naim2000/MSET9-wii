#include <stdio.h>
#include <unistd.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>

#include "sd.h"
#include "fatfs/ff.h"
#include "pad.h"
#include "video.h"

static bool sdMounted = false;
static const DISC_INTERFACE* sdmc = &__io_wiisd;
static FATFS fs = {};

bool SDMount(void) {
	if (sdMounted) return true;

	sdmc->startup();

	char spin[] = "|/-\\";
	char* p = spin;
	bool inserted = sdmc->isInserted();
	while (!inserted) {
		printf(pWarn "Please insert your SD card. [%c]\r", *p++);
		scanpads();
		if (buttons_down(WPAD_BUTTON_HOME)) break;

		if (!*p) {
			p = spin;
			sdmc->startup();
			inserted = sdmc->isInserted();
		}

		usleep(100000);
	}
	clearln();

	if (!inserted) {
		printf(pBad "No SD card inserted!\n");
		return false;
	}

	printf(pInfo "Mounting sdmc:/ ...\r");
	FRESULT fres = f_mount(&fs, "sdmc:/", true);
	if (fres == FR_OK) {
		unsigned long freeSpace = (fs.free_clst * fs.csize) * 0x200;
		unsigned long totalSpace = ((fs.n_fatent - 2) * fs.csize) * 0x200;

		puts(pGood "Mounting sdmc:/ OK!");
		sdMounted = true;

		if (totalSpace < 0x80000000) // 2GB
		{
			printf(pWarn "Your SD card is under 2GB? (%luMB)\n", totalSpace / 0x100000);
			sleep(2);
		}


		if (freeSpace < 0x1000000) // 16MB
		{
			printf(pBad "Error #06: Insufficient SD card space!\n"
				   pBad "At least 16MB is required, you have %.2fMB!", freeSpace / (float)0x100000);

			SDUnmount();
		}

	}
	else {
		printf(pBad "Mounting sdmc:/ failed! (%i)\n", fres);
	}

	return sdMounted;
}

void SDUnmount(void) {
	if (sdMounted) f_unmount("sdmc:/");
	sdmc->shutdown();
	sdMounted = false;
}
