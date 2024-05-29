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
// TODO
// static uint32_t sdcard_CID[4] __aligned(0x20);

bool SDMount(void) {
	if (sdMounted) return true;

	sdmc->startup();


	const char spin[] = "|/-\\";
	const char* p = spin;
	bool inserted = sdmc->isInserted();
	while (!inserted) {
		printf(pWarn "Insert your SD card, then press (A) to continue. [%c]\r", *p++);
		scanpads();
		if (buttons_down(WPAD_BUTTON_A)) break;

		if (!*p) p = spin;

		usleep(100000);
	}
	clearln(0);

	if (!sdmc->isInserted()) {
	//	puts(pWarn "Operation cancelled by user...");
		puts(pBad "No SD card inserted?");
		return false;
	}

	printf(pInfo "Mounting SD card ...\r");
	FRESULT fres = f_mount(&fs, "sdmc:/", true);
	if (fres == FR_OK) {
		unsigned long freeSpace = (fs.free_clst * fs.csize);
		unsigned long totalSpace = ((fs.n_fatent - 2) * fs.csize);

		puts(pGood "Mounting SD card OK!");
		sdMounted = true;
		// WiiSD_GetCID(&sdcard_CID);

		if (totalSpace < 0x400000) // 2GB (4M sectors)
		{
			printf(pWarn "Your SD card is under 2GB? (%luMB)\n", totalSpace / 0x800);
			sleep(2);
		}


		if (freeSpace < 0x8000)
		{
			printf(pBad "Error #06: Insufficient SD card space!\n"
					// Double, because if we inject MSET9 and dip below 16MB wel'll have trouble uninjecting it
				   pBad "At least 512 blocks (32MB) are required, you have %lu!", freeSpace / 0x100);

			SDUnmount();
		}

	}
	else {
		printf(pBad "Mounting SD card failed! (%i)\n", fres);
	}

	return sdMounted;
}

void SDUnmount(void) {
	if (sdMounted) {

		f_unmount("sdmc:/");
	// sdmc->shutdown();
		puts(pInfo "Unmounted SD card.");
		sdMounted = false;
	}
}

bool SDRemount(void) {
	SDUnmount();

	const char spin[] = "|/-\\";
	const char* p = spin;
	bool inserted = sdmc->isInserted();
	while (inserted) {
		printf(pWarn "Eject your SD card! [%c] (Press HOME to cancel)\r", *p++);
		scanpads();
		if (buttons_down(WPAD_BUTTON_HOME)) break;

		if (!*p) {
			p = spin;
			sdmc->startup();
			inserted = sdmc->isInserted();
		}

		usleep(100000);
	}

	clearln(0);
	if (inserted) return false;

	puts(pGood "Ejected SD card.");
	sleep(5);

	return SDMount();
}
