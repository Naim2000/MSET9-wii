#include <stdio.h>
#include <unistd.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>

#include "sd.h"
#include "pad.h"
#include "video.h"

static bool sdMounted = false;
static const DISC_INTERFACE* sdmc = &__io_wiisd;

bool SDMount(void) {
	if (sdMounted) return true;

	sdmc->startup();

	bool b = true;
	bool inserted = sdmc->isInserted();
	while (!inserted) {
		printf("%sPlease insert your SD card.\r", b? pWarn : " ");
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

	if (!inserted) return false;

	sdMounted = fatMountSimple("sdmc", sdmc);
	if (!sdMounted) {
		puts(pBad "SD mount failed!");
		return false;
	}

	chdir("sdmc:/");
	return true;
}

void SDUnmount(void) {
	if (sdMounted) fatUnmount("sdmc");
	sdmc->shutdown();
	sdMounted = false;
}
