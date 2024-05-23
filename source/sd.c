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
	return (sdMounted = (f_mount(&fs, "sdmc:/", 1) == FR_OK));

}

void SDUnmount(void) {
	if (sdMounted) f_unmount("sdmc:/");
	sdmc->shutdown();
	sdMounted = false;
}
