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

	bool b = true;
	bool inserted = sdmc->isInserted();
	while (!inserted) {
		wprintf(L"%lsPlease insert your SD card.\r", b? pWarn : L"  ");
		b = !b;
		scanpads();
		if (buttons_down(WPAD_BUTTON_HOME)) break;

		if (!b) {
			sdmc->startup();
			inserted = sdmc->isInserted();
		}

		usleep(500000);
	}
	clearln();

	if (!inserted) {
		wprintf(pBad "No SD card inserted!\n");
		return false;
	}

	wprintf(pInfo "Mounting sdmc:/ ...\r");
	FRESULT fres = f_mount(&fs, L"sdmc:/", 1);
	if (fres == FR_OK) {
		wprintf(pGood "Mounting sdmc:/ OK!\n");
		sdMounted = true;
	}
	else {
		wprintf(pBad "Mounting sdmc:/ failed! (%i)\n", fres);
		sdMounted = false;
	}

	return sdMounted;
}

void SDUnmount(void) {
	if (sdMounted) f_unmount(L"sdmc:/");
	sdmc->shutdown();
	sdMounted = false;
}
