#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

static int SelectionMenu(const char* const options[], int count) {
	int posX, posY, selected = 0;
	CON_GetPosition(&posX, &posY);

	while (true) {
		printf("\x1b[%i;0H", posY);
		for (int i = 0; i < count; i++)
			printf("	%s %s%s\n", i == selected ? "\x1b[47;1m\x1b[30m>>" : "  ", options[i], i == selected ? "\x1b[40m\x1b[39m" : "");

		wait_button(0);

		if (buttons_down(WPAD_BUTTON_UP)) {
			if (!selected--)
				selected = count - 1;
			continue;
		}

		else if (buttons_down(WPAD_BUTTON_DOWN)) {
			if (++selected == count)
				selected = 0;
			continue;
		}

		else if (buttons_down(WPAD_BUTTON_A)) {
			selected++;
			break;
		}

		else if (buttons_down(WPAD_BUTTON_HOME)) {
			selected = -1;
			break;
		}
	}

	putchar('\n');
	putchar('\n');
	return selected;
}

int SelectConsoleVer(void)
{
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
		"New 3DS/New 3DS Xl/New 2DS XL, 11.4 up to 11.7"
	};

	return SelectionMenu(options, 4);
}

void SanityReport(void)
{
	if (!mset9.homeExtdataOK) {
		prbad("HOME menu extdata: Missing!");
		prinfo("Please power on your console with your SD inserted, then try again.");
		prinfo("If this does not work, power it on while holding L+R+Down+B.\n");
	}
	else prgood("HOME menu extdata: OK!");

	if (!mset9.miiExtdataOK) {
		prbad("Mii Maker extdata: Missing!");
		prinfo("Please power on your console with your SD inserted,");
		prinfo("then launch Mii Maker.\n");
	}
	else prgood("Mii Maker extdata: OK!");

	if (!mset9.titleDbsOK) {
		prbad("Title database: Not initialized!");

		prinfo("Please power on your console with your SD inserted,");
		prinfo("open System Setttings, navigate to Data Management");
		prinfo(" -> Nintendo 3DS -> Software, then select Reset.\n");
	}
	else prgood("Title database: OK!");

}

int main(void) {
	initpads();
	if (!SDMount()) goto exit;

	PrintHeader();

	if (!MSET9Start() || !MSET9SanityCheckA()) goto exit;

	PrintHeader();
	if (!mset9.hasHaxID1) { // Create hax ID1
		int consoleVer = SelectConsoleVer();
		if (consoleVer > 0) {
			PrintHeader();
			puts(
				"\x1b[41;39m=== /!\\ DISCLAIMER /!\\ ===\x1b[40m\x1b[39m\n\n"

				"This process will temporarily reset all your 3DS data.\n"
				"All your applications and themes will disappear.\n"
				"This is perfectly normal, and if everything goes right. it will re-appear\n"
				"at the end of the process.\n\n"

				"In any case, it is highly recommended to make a backup of your SD card's\n"
				"contents to a folder on your PC.\n"
				"(Especially the 'Nintendo 3DS' folder.)\n"
			);

			sleep(7);

			int response = SelectionMenu((const char*[]) { "I DO NOT ACCEPT", "I ACCEPT" }, 2);
			if (response == 2)
				MSET9CreateHaxID1((MSET9Version)consoleVer);
		}


		goto exit;
	}
/*
	else if (mset9.consoleVer != consoleVer) {
		puts(pBad "Error #03: Don't change console model/version during MSET9!\n");

		printf("Earlier, you selected: '%s'!\n", consoleNames[mset9.consoleVer]);
		printf("Are you sure you want to switch to: '%s'?\n\n", consoleNames[consoleVer]);

		sleep(3);


		int response = SelectionMenu((const char*[]){consoleNames[consoleVer], consoleNames[mset9.consoleVer], "Cancel"}, 3);
		switch (response) {
		case 0:
			MSET9CreateHaxID1((MSET9Version)consoleVer);
		case 1:
			break;
		default:
			goto exit;
		}
	}
*/
	while (true) {
		MSET9SanityCheckB();

		if (mset9.hasTriggerFile) {
			MSET9Injection(false);
/*
			puts(pWarn "Try again?\n\n"

				 pInfo "Press (+) to try again.\n"
				 pInfo "Press (-) to remove MSET9.\n");

			wait_button(WPAD_BUTTON_PLUS | WPAD_BUTTON_MINUS);
			if (buttons_down(WPAD_BUTTON_MINUS)) {
				MSET9Remove();
				goto exit;
			}
*/
			int response = SelectionMenu((const char*[]) { "Try again", "I got boot9strap now, remove MSET9" }, 2);
			switch (response) {
				case 1:
					break;
				case 2:
					MSET9Remove();
				default:
					goto exit;
			}
		}
		else if (mset9.homeExtdataOK && mset9.miiExtdataOK && mset9.titleDbsOK) {
			prinfo("Selected console version: %s", consoleNames[mset9.consoleVer-1]);
			prwarn("Inject MSET9 now?");

			int response = SelectionMenu((const char* []) { "Yes, I am currently in Extra Data", "No, not yet", "Change console version", "Remove MSET9", "Exit" }, 5);

			switch (response) {
				case 1:
					MSET9Injection(true);
				case 2:
					break;
				case 3:
					response = SelectConsoleVer();
					if (response >= 0)
						MSET9CreateHaxID1((MSET9Version)response);
					break;
				case 4:
					MSET9Remove();
				default:
					goto exit;

			}
		}
		else {
			PrintHeader();
			SanityReport();

			int response = SelectionMenu((const char* []) { "I'll fix that", "Remove MSET9", "Exit" }, 3);
			switch (response) {
				case 2:
					MSET9Remove();
				default:
					goto exit;
				case 1:
					break;
			}
		}

		if (!SDRemount()) break;
		PrintHeader();
		int ret = MSET9Start();
		if (!ret) break;
	}

exit:
	SDUnmount();
	puts("\nPress HOME to exit.");
	wait_button(WPAD_BUTTON_HOME);
	return 0;
}
