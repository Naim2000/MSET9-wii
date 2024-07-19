#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>

#include "mset9.h"
#include "video.h"
#include "fsop.h"

#define ID1backupTag "_user-id1"
#define MSET9TriggerFile "002F003A.txt"

enum {
	dbsSize = 0x31E400,
};

enum {
	REGION_USA,
	REGION_EUR,
	REGION_JPN,
	REGION_CHN,
	REGION_KOR,
	REGION_TWN,

	NBR_REGIONS
};

typedef const struct N3DSRegion
{
	char name[4];
	uint32_t homeMenuExtData;
	uint32_t MiiMakerExtData;
	uint32_t MiiPlazaExtData;
} N3DSRegion;

/*
typedef const struct haxOffsets
{
	uint32_t fopen_offset;
	uint32_t fread_offset;
} haxOffsets;
*/

static N3DSRegion N3DSRegions[NBR_REGIONS] = {
	{ "USA", 0x8F, 0x217, 0x218 },
	{ "EUR", 0x98, 0x227, 0x228 },
	{ "JPN", 0x82, 0x207, 0x208 },
	{ "CHN", 0xA1, 0x267, -1 },
	{ "KOR", 0xA9, 0x277, -1 },
	{ "TWN", 0xB1, 0x287, -1 },
};

const char* const haxid1[4] = {

	// Old 3DS 11.8<->11.17
	"\xef\xbf\xbf\xef\xab\xbf\xe9\xa4\x91\xe4\xa0\x87\xe4\x9a\x85\xe6\x95\xa9\xea\x84\x88\xe2\x88\x81\xe4\xac\x85\xe4\x9e\x98\xe4\x99\xa8\xe4\x99\x99\xea\xab\x80\xe1\xb0\x97\xe4\x99\x83\xe4\xb0\x83\xe4\x9e\xa0\xe4\x9e\xb8\xe9\x80\x80\xe0\xa0\x8a\xea\x81\xb1\xe0\xa0\x85\xec\xba\x99\xe0\xa0\x84sdmc\xe9\x80\x80\xe0\xa0\x8a""b9",

	// New 3DS 11.8<->11.17
	"\xef\xbf\xbf\xef\xab\xbf\xe9\xa4\x91\xe4\xa0\x87\xe4\x9a\x85\xe6\x95\xa9\xea\x84\x88\xe2\x88\x81\xe4\xac\x85\xe4\x9e\x98\xe4\x99\xa8\xe4\x99\x99\xea\xab\x80\xe1\xb0\x97\xe4\x99\x83\xe4\xb0\x83\xe4\x9e\xa0\xe4\x9e\xb8\xe9\x80\x80\xe0\xa0\x8a\xea\x81\xb1\xe0\xa0\x85\xec\xb9\x9d\xe0\xa0\x84sdmc\xe9\x80\x80\xe0\xa0\x8a""b9",

	// Old 3DS 11.4<->11.7
	"\xef\xbf\xbf\xef\xab\xbf\xe9\xa4\x91\xe4\xa0\x87\xe4\x9a\x85\xe6\x95\xa9\xea\x84\x88\xe2\x88\x81\xe4\xac\x85\xe4\x9e\x98\xe4\x99\xa8\xe4\x99\x99\xea\xab\x80\xe1\xb0\x97\xe4\x99\x83\xe4\xb0\x83\xe4\x9e\xa0\xe4\x9e\xb8\xe9\x80\x80\xe0\xa0\x8a\xe9\xb9\x89\xe0\xa0\x85\xec\xb2\x99\xe0\xa0\x84sdmc\xe9\x80\x80\xe0\xa0\x8a""b9",

	// New 3DS 11.4<->11.7
	"\xef\xbf\xbf\xef\xab\xbf\xe9\xa4\x91\xe4\xa0\x87\xe4\x9a\x85\xe6\x95\xa9\xea\x84\x88\xe2\x88\x81\xe4\xac\x85\xe4\x9e\x98\xe4\x99\xa8\xe4\x99\x99\xea\xab\x80\xe1\xb0\x97\xe4\x99\x83\xe4\xb0\x83\xe4\x9e\xa0\xe4\x9e\xb8\xe9\x80\x80\xe0\xa0\x8a\xe9\xb9\x85\xe0\xa0\x85\xec\xb2\x81\xe0\xa0\x84sdmc\xe9\x80\x80\xe0\xa0\x8a""b9"
};


struct MSET9 mset9 = {};

static bool is3DSID(const char* name) {
	if (strlen(name) != 32) return false;

	uint32_t idparts[4];
	return (sscanf(name, "%08x%08x%08x%08x", &idparts[0], &idparts[1], &idparts[2], &idparts[3]) == 4);
}

static bool isBackupID1(const char* name) {
	return (strstr(name, ID1backupTag)) == (name + 32);
}

static bool isHaxID1(const char* name) {
	return (strstr(name, "sdmc") != NULL);
}

int MSET9Start(void) {
	char path[256] = "Nintendo 3DS/";
	FRESULT fres;
	DIR dp = {};
	FILINFO st[1] = {};

	memset(&mset9, 0, sizeof(mset9));

	fres = f_opendir(&dp, path);
	if (fres == FR_NO_PATH) {
		printf(pBad "Error #01: Nintendo 3DS folder not found!\n"
			   pBad "Is your console reading the SD card?\n");
		return false;
	}
	else if (fres != FR_OK) {
		printf(pBad "Failed to open Nintendo 3DS folder (!?) (%i)\n", fres);
		return false;
	}

	int foundID0 = 0;
	while ((fres = f_readdir(&dp, st)) == FR_OK && st->fname[0]) {
		if (!(st->fattrib & AM_DIR)) {
			printf(pWarn "Found a file in the Nintendo 3DS folder? %s\n", st->fname);
			continue;
		}

		if (is3DSID(st->fname)) {
			printf(pInfo "Found ID0: %s\n", st->fname);
			strncpy(mset9.ID[0], st->fname, 32);
			foundID0++;
		}
	}
	f_closedir(&dp);

	if (!foundID0) {
		printf(pBad "Error #07: You have...... no ID0?\n"
			   pBad "Is your console reading the SD card?\n");

		return false;
	}
	else if (foundID0 > 1) {
		printf(pBad  "Error #07: You have multiple (%i) ID0's!\n"
			   pInfo "Consult: https://wiki.hacks.guide/wiki/3DS:MID0", foundID0);

		return false;
	}

	sprintf(strrchr(path, '/'), "/%.32s/", mset9.ID[0]);
	fres = f_opendir(&dp, path);
	if (fres != FR_OK) {
		printf(pBad "Failed to open ID0 folder (!?) (%i)\n", fres);
		return false;
	}

	int foundID1 = 0;
	int injectedConsoleVer = -1;
	while ((fres = f_readdir(&dp, st)) == FR_OK && st->fname[0]) {
		if (!(st->fattrib & AM_DIR)) {
			printf(pWarn "Found a file in the ID0 folder? %s\n", st->fname);
			continue;
		}

		if (is3DSID(st->fname) || isBackupID1(st->fname)) {
			printf(pInfo "Found ID1: %s\n", st->fname);
			strncpy(mset9.ID[1], st->fname, 32);
			mset9.hasBackupID1 = isBackupID1(st->fname);
			foundID1++;
		}
		else if (isHaxID1(st->fname)) {
			char* ptr = strrchr(path, '/');
			sprintf(ptr, "/%s/", st->fname);

			if (mset9.hasHaxID1) {
				puts(pWarn "Duplicate hax ID1!? Man what the h*ll");
				if ((fres = f_rmdir_r(path)) != FR_OK)
					printf(pWarn "Failed to remove duplicate hax ID1 (%i)\n", fres);
			}
			else {
				for (int i = 0; i < 4; i++) {
					if (!strcmp(haxid1[i], st->fname)) {
						injectedConsoleVer = i;
						break;
					}
				}

				if (injectedConsoleVer >= 0) {
					printf(pInfo "Found hacked ID1 (type %i)\n", injectedConsoleVer);
					strcat(path, "/extdata/" MSET9TriggerFile);
					mset9.consoleVer = (MSET9Version)injectedConsoleVer;
					mset9.hasHaxID1 = true;
					mset9.hasTriggerFile = (f_stat(path, 0) == FR_OK);
				} else {
					puts(pWarn "Unrecognized hax ID1. Goodbye");
					if ((fres = f_rmdir_r(path)) != FR_OK)
						printf(pWarn "Failed to unknown duplicate hax ID1 (%i)\n", fres);
				}
			}

			*ptr = 0;
		}
		else {
			printf(pWarn "Unrecognized folder in ID0 %s\n", st->fname);
		}
	}
	f_closedir(&dp);

	if (!foundID1) {
		puts(pBad "Error #12: You have...... no ID1?\n"
			 pBad "Is your console reading the SD card?");

		return false;
	}
	else if (foundID1 > 1) {
		printf(pBad "Error #12: You have multiple (%i) ID1's!\n"
			   pBad "Consult: https://wiki.hacks.guide/wiki/3DS:MID1\n", foundID1);

		return false;
	}

	mset9.setup = true;
	return true;
}

bool MSET9SanityCheckA(void) {
//	puts(pInfo "Checking extracted files...");
	if (!CheckFile("/boot9strap/boot9strap.firm", 0, true)
	+	!CheckFile("/boot.firm", 0, false)
	+	!CheckFile("/boot.3dsx", 0, false)
	+	!CheckFile("/b9", 0, false)
	+	!CheckFile("/SafeB9S.bin", 0, false))
	{
		puts(pBad  "Error #08: One or more files are missing or malformed!");
		puts(pInfo "Please re-extract the MSET9 zip file, \n"
			 pInfo "overwriting any files when prompted.");

		return false;
	}

	puts(pGood "Extracted files look good!");
	return true;
}

bool MSET9CreateHaxID1(MSET9Version ver) {
	char path[256] = "/Nintendo 3DS/";
	FRESULT fres;

	if (!mset9.setup) return false;

	sprintf(strrchr(path, '/'), "/%.32s/", mset9.ID[0]);

	fres = f_chdir(path);
	if (fres != FR_OK) {
		printf(pBad "f_chdir failed? (%i)\n", fres);
		return false;
	}

	if (mset9.hasHaxID1) {
		if (ver == mset9.consoleVer) return true;

		puts(pInfo "Renaming hacked ID1...");
		fres = f_rename(haxid1[mset9.consoleVer], haxid1[ver]);
	} else {
		puts(pInfo "Creating hacked ID1...");
		fres = f_mkdir(haxid1[ver]);
	}

	if (fres == FR_OK) {
		f_chdir(haxid1[ver]);
		f_mkdir("dbs");
		f_dummy("dbs/title.db",  false);
		f_dummy("dbs/import.db", false);
		f_chdir(path);
	} else {
		printf(pBad "Failed to create hax ID1 (%i)\n", fres);
	}

	if (!mset9.hasBackupID1) {
		char ID1backupName[42];
		strcpy(ID1backupName, mset9.ID[1]);
		strcat(ID1backupName, ID1backupTag);

		puts(pInfo "Backing up original ID1...");
		fres = f_rename(mset9.ID[1], ID1backupName);
		if (fres != FR_OK)
			printf("f_rename failed? (%i)\n", fres);
	}

	f_chdir("");
	return (fres == FR_OK);
}

bool MSET9SanityCheckB(void) {
	char path[256];

	if (!mset9.hasHaxID1) return -1;

	sprintf(path, "/Nintendo 3DS/%.32s/%s/", mset9.ID[0], haxid1[mset9.consoleVer]);
	FRESULT fres = f_chdir(path);
	if (fres != FR_OK) {
		printf("f_chdir failed? (%i)\n", fres);
		return false;
	}

//	puts(pNote "Performing sanity checks...");

	puts(pInfo "Checking databases...");
	if (!CheckFile("dbs/import.db", dbsSize, false)
	+	!CheckFile("dbs/title.db",  dbsSize, false))
	{
		f_mkdir("dbs");
		f_dummy("dbs/title.db", false);
		f_dummy("dbs/import.db", false);
	}
	else mset9.titleDbsOK = true;

	puts(pInfo "Looking for extdata...");
	char extDataPath[30];

	for (N3DSRegion* rgn = N3DSRegions; rgn < N3DSRegions + NBR_REGIONS; rgn++) {
		if (!mset9.homeExtdataOK) {
			sprintf(extDataPath, "extdata/00000000/%08x", rgn->homeMenuExtData);
			mset9.homeExtdataOK = (f_stat(extDataPath, 0) == FR_OK);
		}

		if (!mset9.miiExtdataOK) {
			sprintf(extDataPath, "extdata/00000000/%08x", rgn->MiiMakerExtData);
			mset9.miiExtdataOK  = (f_stat(extDataPath, 0) == FR_OK);
		}
	}

	f_chdir("");
	return mset9.titleDbsOK && mset9.miiExtdataOK && mset9.homeExtdataOK;
}

bool MSET9Injection(bool create) {
	char path[256];
	FRESULT fres;
//	FIL fp;

	if (!mset9.hasHaxID1) return false;

	sprintf(path, "/Nintendo 3DS/%.32s/%s/extdata/" MSET9TriggerFile, mset9.ID[0], haxid1[mset9.consoleVer -1]);

	if (!create) {
		puts(pInfo "Removing trigger file...");
		fres = f_unlink(path);
		if (fres != FR_OK) {
			printf(pBad "f_unlink failed! (%i)\n", fres);
			return false;
		}
	} else {
		puts(pInfo "Injecting trigger file...");
		fres = f_dummy(path, true);
		if (fres != FR_OK) {
			printf(pBad "f_dummy failed! (%i)\n", fres);
			return false;
		}
	}

/*
	UINT written;
	fres = f_write(&fp, "arm9 \"security\" processor vs. text file in extdata:", 52, &written);
	f_close(&fp);
*/

	return true;
}

void MSET9Remove(void) {
	char path[256];

	if (!mset9.setup) return;

	puts(pInfo "Removing MSET9...");
	sprintf(path, "0:/Nintendo 3DS/%.32s/", mset9.ID[0]);

	FRESULT fres = f_chdir(path);
	if (fres != FR_OK) {
		printf("f_chdir failed? (%i)\n", fres);
		return;
	}

	if (mset9.hasHaxID1) {
		puts(pInfo "Removing hacked ID1...");
		fres = f_rmdir_r(haxid1[mset9.consoleVer -1]);
		if (fres != FR_OK)
			printf("f_rmdir failed? (%i)\n", fres);
		else
			mset9.hasHaxID1 = false;
	}

	if (mset9.hasBackupID1) {
		char ID1backupName[42];
		strcpy(ID1backupName, mset9.ID[1]);
		strcat(ID1backupName, ID1backupTag);

		fres = f_rename(ID1backupName, mset9.ID[1]);
		if (fres != FR_OK)
			printf("f_rename failed? (%i)\n", fres);
		else
			mset9.hasBackupID1 = false;
	}
}
