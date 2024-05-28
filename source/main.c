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

static void PrintHeader(void) {
	clear();


	puts("MSET9-Wii installer by thepikachugamer\n"
		 "MSET9 exploit by zoogie - https://github.com/zoogie/MSET9");

	clearln('\315');
}

int main(void) {
	initpads();
	if (!SDMount()) goto exit;

	if (!MSET9Start()) goto exit;

	int consoleVer = 0;

	PrintHeader();
	puts("\n"
		 "	-+========= What is your console model and version? =========+-\n"
		 "	Console Model: Old 3DS has 2 shoulder buttons (L, R)\n"
		 "	               New 3DS has 4 shoulder buttons (L, R, ZL, ZR)\n\n"

		 "	Console Version: Displayed on the main page of System Settings.\n"
		 "	                 Eg. \"Ver. 11.17.0-50J\"\n"
	);
	const char* const options[4] = {
		"Old 3DS/Old 3DS XL/Old 2DS,    11.8 up to 11.17 (latest!)",
		"New 3DS/New 3DS XL/New 2DS XL, 11.8 up to 11.17 (latest!)",
		"Old 3DS/Old 3DS XL/Old 2DS,    11.4 up to 11.7",
		"New 3DS/New 3DS Xl/New 2DS XL, 11.4 up to 11.7"};

	const char* const shortNames[4] = {
		"Old 3DS, 11.8 up to 11.17",
		"New 3DS, 11.8 up to 11.17",
		"Old 3DS, 11.4 up to 11.7",
		"New 3DS, 11.4 up to 11.7"
	};

	int posX, posY;
	CON_GetPosition(&posX, &posY);

	while (true) {

		printf("\x1b[%i;0H", posY);
		for (int ii = 0; ii < 4; ii++)
			printf("	%s %s\x1b[40m\x1b[39m\n", ii == consoleVer ? "\x1b[47;1m\x1b[30m>>" : "  ", options[ii]);

		wait_button(0);

		if (buttons_down(WPAD_BUTTON_UP)) { if (!consoleVer--) consoleVer = 3; continue; }

		else if (buttons_down(WPAD_BUTTON_DOWN)) { if (++consoleVer == 4) consoleVer = 0; continue; }

		else if (buttons_down(WPAD_BUTTON_A)) { consoleVer++; break; }

		else if (buttons_down(WPAD_BUTTON_B | WPAD_BUTTON_HOME)) { consoleVer = 0; break; }
	}

	if (!consoleVer) goto exit;

	MSET9SetConsoleVer((MSET9Version)consoleVer);

	while (true) {
		PrintHeader();
		if (MSET9SanityCheck()) {
			printf(pNote "Selected console version: %s\n", shortNames[consoleVer - 1]);
			printf(pNote "Inject MSET9 now?\n\n"

				   pInfo "Press (A) to confirm.\n"
				   pInfo "Press [B] to cancel.\n");
			wait_button(WPAD_BUTTON_A | WPAD_BUTTON_B | WPAD_BUTTON_HOME);
			if (buttons_down(WPAD_BUTTON_A)) MSET9Injection();
		}
		if (!SDRemount()) break;
		PrintHeader();
		int ret = MSET9Start();
		if (!ret) break;
		else if (ret == 2) {
			printf(pWarn "Try again?\n\n"

				   pInfo "Press (A) to confirm.\n"
				   pInfo "Press [B] to cancel.\n");
			wait_button(WPAD_BUTTON_A | WPAD_BUTTON_B | WPAD_BUTTON_HOME);
			if (!buttons_down(WPAD_BUTTON_A)) break;
		}
	}

exit:
	SDUnmount();
	puts("\nPress HOME to exit.");
	wait_button(WPAD_BUTTON_HOME);
	return 0;
}
