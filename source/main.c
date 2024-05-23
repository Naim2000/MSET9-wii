#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "video.h"
#include "pad.h"
#include "sd.h"
#include "fatfs/ff.h"
#include "mset9.h"

__weak_symbol __printflike(1, 2)
void OSReport(const char* fmt, ...) {}

int main(void) {
	puts(
		"MSET9-Wii installer by thepikachugamer\n"
		"MSET9 exploit by zoogie https://github.com/zoogie/MSET9\n" );

	initpads();
	if (!SDMount()) goto exit;

	MSET9Start(1);

	if (!MSET9SanityCheck()) goto exit;

	puts("Inject MSET9 now?");
	wait_button(0);
	if (buttons_down(WPAD_BUTTON_PLUS)) MSET9Injection();

exit:
	SDUnmount();
	puts("\nPress HOME to exit.");
	wait_button(WPAD_BUTTON_HOME);
	return 0;
}
