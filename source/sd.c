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

	const char spin[] = "|/-\\";
	const char* p = spin;
	bool inserted = sdmc->isInserted();
	while (!inserted) {
		printf("Please insert your SD card. [%c]\r", *p++);
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

	if (!inserted) {
		printf("Operation cancelled by user\n");
		return false;
	}

	printf("Mounting SD card ... ");
	// usleep(1000000);
	FRESULT fres = f_mount(&fs, "", true);
	if (fres == FR_OK) {
		unsigned long freeSpace = (fs.free_clst * fs.csize);
		unsigned long totalSpace = ((fs.n_fatent - 2) * fs.csize);

		puts("OK!");
		sdMounted = true;

		if (totalSpace < 0x400000) // 2GB (4M sectors)
		{
			prwarn("Your SD card is under 2GB? (%luMiB)\n", totalSpace >> (20 - 9));
			usleep(2000000);
		}

	}
	else {
		printf("Failed! (%i)\n", fres);
	}

	return sdMounted;
}

void SDUnmount(void) {
	if (sdMounted) {
		f_unmount("");
		// sdmc->shutdown();
		prinfo("Unmounted SD card.");
		sdMounted = false;
	}
}

bool SDRemount(void) {
	SDUnmount();

	const char spin[] = "|/-\\";
	const char* p = spin;
	bool inserted = sdmc->isInserted();
	while (inserted) {
		printf("Eject your SD card! [%c]\r", *p++);
		scanpads();
		if (buttons_down(WPAD_BUTTON_HOME)) break;

		if (!*p) {
			p = spin;
			sdmc->startup();
			inserted = sdmc->isInserted();
		}

		usleep(100000);
	}

	sdmc->shutdown();

	clearln(0);
	if (inserted) return false;

	prinfo("SD card ejected.");
	usleep(10000000);

	return SDMount();
}
