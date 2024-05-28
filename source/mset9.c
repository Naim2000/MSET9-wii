#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>

#include "mset9.h"
#include "video.h"
#include "fsop.h"

#define ID1backupTag "_user-id1"

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

struct MSET9 {
	bool ready;
	MSET9Version consoleVer;
	N3DSRegion* region;
	char ID[2][32 +1];
	bool hasBackupID1;
	bool hasHaxID1;
};

static struct MSET9 mset9 = {};

static bool is3DSID(const char* name) {
	if (strlen(name) != 32) return false;

	uint32_t idparts[4];
	return sscanf(name, "%08x%08x%08x%08x", &idparts[0], &idparts[1], &idparts[2], &idparts[3]) == 4;
}

static bool isBackupID1(const char* name) {
	return strstr(name, ID1backupTag) == name + 32;
}

static bool isHaxID1(const char* name) {
	return (bool)strstr(name, "sdmc");
}

int MSET9Start() {
	char path[256] = "sdmc:/Nintendo 3DS";
	FRESULT fres = 0;
	DIR dp = {};
	FILINFO fl = {};

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
	while ((fres = f_readdir(&dp, &fl)) == FR_OK && fl.fname[0]) {
		if (is3DSID(fl.fname)) {
			printf(pInfo "Found ID0: %s\n", fl.fname);
			strncpy(mset9.ID[0], fl.fname, 32);
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

	strcat(path, "/");
	strcat(path, mset9.ID[0]);
	fres = f_opendir(&dp, path);
	if (fres != FR_OK) {
		printf(pBad "Failed to open ID0 folder (!?) (%i)\n", fres);
		return false;
	}

	int foundID1 = 0;
	int injectedConsoleVer = -1;
	char curhaxID1[100] = {};
	while ((fres = f_readdir(&dp, &fl)) == FR_OK && fl.fname[0]) {
		if (is3DSID(fl.fname) || isBackupID1(fl.fname)) {
			printf(pInfo "Found ID1: %s\n", fl.fname);
			strncpy(mset9.ID[1], fl.fname, 32);
			mset9.hasBackupID1 = isBackupID1(fl.fname);
			foundID1++;
		}
		else if (isHaxID1(fl.fname)) {
			injectedConsoleVer = 0;
			strcpy(curhaxID1, fl.fname);
			for (int i = 0; i < 4; i++) {
				if (!strcmp(haxid1[i], fl.fname)) {
					injectedConsoleVer = i + 1;
					break;
				}
			}

			printf(pInfo "Found injected MSET9, #%i\n", injectedConsoleVer);
		}
		else {
			printf(pWarn "Unrecognized ID1 %s\n", fl.fname);
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

	if (mset9.hasBackupID1) {
		puts(pNote "Found backed up ID1, let's rename it back for you");
		char backupID1name[50];
		strcpy(backupID1name, mset9.ID[1]);
		strcat(backupID1name, ID1backupTag);
		f_chdir(path);
		if ((fres = f_rename(backupID1name, mset9.ID[1])) != FR_OK) {
			printf(pWarn "Failed to restore backed up ID1 ..? (%i)\n", fres);
		}
		f_chdir("sdmc:/");
	}

	if (injectedConsoleVer >= 0) {
		puts(pNote "Found hax ID1, let's remove it for you");
		f_chdir(path);
		if ((fres = f_rmdir_r(curhaxID1)) != FR_OK) {
			printf(pWarn "Failed to remove hax ID1 ..? (%i)\n", fres);
		}
		f_chdir("sdmc:/");

		puts(pGood "MSET9 has been removed!");
		return 2;
	}

	mset9.ready = true;
	return true;
}

bool MSET9SetConsoleVer(MSET9Version consoleVer) {
	if (!mset9.ready) return false;

	mset9.consoleVer = consoleVer;
	return true;
}

bool MSET9SanityCheck(void) {
	char path[256];

	if (!mset9.ready) return false;

	puts(pNote "Performing sanity checks...");

	puts(pInfo "Checking extracted files...");
	if (!CheckFile("/boot9strap/boot9strap.firm", 0, true)
	||	!CheckFile("/boot.firm", 0, false)
	||	!CheckFile("/boot.3dsx", 0, false)
	||	!CheckFile("/b9", 0, false)
	||	!CheckFile("/SafeB9S.bin", 0, false))
	{
		puts(pBad  "Error #08: One or more files are missing or malformed!");
		puts(pInfo "Please re-extract the MSET9 zip file, overwriting any files when prompted.");

		return false;
	}
	puts(pGood "Extracted files look good!");

	sprintf(path, "sdmc:/Nintendo 3DS/%.32s/%.32s/", mset9.ID[0], mset9.ID[1]);
	FRESULT fres = f_chdir(path);
	if (fres != FR_OK) {
		printf("f_chdir failed? (%i)\n", fres);
		return false;
	}

	puts(pInfo "Checking databases...");
	if (!CheckFile("dbs/import.db", dbsSize, false)
	||	!CheckFile("dbs/title.db",  dbsSize, false))
	{
		puts(pNote "Information #10: No title database!\n");

		FRESULT fres = f_mkdir("dbs");
		if (fres != FR_OK && fres != FR_EXIST) {
			printf("f_mkdir failed! (%i)\n", fres);
			return false;
		}

		f_dummy("dbs/title.db", true);
		f_dummy("dbs/import.db", true);

		puts(pNote "Created empty databases.\n\n"

			 pInfo "Please initialize the title database by navigating to\n"
			 pInfo "	System Settings -> Data Management -> Nintendo 3DS\n"
			 pInfo "	-> Software -> Reset, then re-run the installer.\n\n"

			 pInfo "Visual aid: \n"
			 pInfo "https://3ds.hacks.guide/images/screenshots/database-reset.jpg"
		);

		return false;
	}
	puts(pGood "Databases look good!");

	puts(pInfo "Looking for HOME menu extdata...");
	char extDataPath[30];

	for (int i = 0; i < NBR_REGIONS; i++) {
		N3DSRegion* rgn = N3DSRegions + i;

		sprintf(extDataPath, "extdata/00000000/%08x", rgn->homeMenuExtData);
		if (f_stat(extDataPath, 0) == FR_OK) {
			printf(pGood "Found %s HOME menu extdata!\n", rgn->name);
			mset9.region = rgn;
			break;
		}
	}

	if (!mset9.region) {
		puts(pBad "Error #04: No HOME menu extdata found!\n\n"

			pNote "How to reset HOME menu management info:\n"
			pNote "Power on your console while holding L+R+Down+B.\n"
		);

		return false;
	}

	puts(pInfo "Looking for Mii Maker extdata...");
	sprintf(extDataPath, "extdata/00000000/%08x", mset9.region->MiiMakerExtData);
	if (f_stat(extDataPath, 0) != FR_OK) {
		puts(pBad "Error #05: No Mii Maker extdata found!");

		sprintf(strrchr(extDataPath, '/'), "/%08x", mset9.region->MiiPlazaExtData);
		if (f_stat(extDataPath, 0) == FR_OK) {
			puts(pWarn "Please make sure you are opening Mii Maker and not Mii Plaza!");
		}

		return false;
	}
	puts(pGood "Found Mii Maker extdata!");

	f_chdir("sdmc:/");
	return true;
}

bool MSET9Injection(void) {
	char path[256], path2[256];
	FRESULT fres;
	FIL fp;

	if (!mset9.ready || !mset9.consoleVer) return false;

	puts(pNote "Performing injection...");

	sprintf(path, "sdmc:/Nintendo 3DS/%.32s/", mset9.ID[0]);
	f_chdir(path);

	// 1. Create hax ID1
	const char* thishaxID1 = haxid1[mset9.consoleVer - 1];
	sprintf(path,  "%s/", thishaxID1);
	sprintf(path2, "%s/", mset9.ID[1]);

	puts(pInfo "Creating hax id1...");
	fres = f_mkdir(path);
	if (fres != FR_OK) {
		printf(pBad "f_mkdir #1 failed! (%i)\n", fres);
		return false;
	}

	// 2. Copy Mii maker & HOME menu extdata over
	puts(pInfo "Copying extdata over...");

	sprintf(strchr(path, '/'),  "/extdata/");
	sprintf(strchr(path2, '/'), "/extdata/");

	fres = f_mkdir(path);
	if (fres != FR_OK) {
		printf(pBad "f_mkdir #2 failed! (%i)\n", fres);
		return false;
	}

	strcat(path,  "00000000/");
	strcat(path2, "00000000/");

	fres = f_mkdir(path);
	if (fres != FR_OK) {
		printf(pBad "f_mkdir #3 failed! (%i)\n", fres);
		return false;
	}

	sprintf(strrchr(path, '/'),  "/%08x", mset9.region->homeMenuExtData);
	sprintf(strrchr(path2, '/'), "/%08x", mset9.region->homeMenuExtData);

	fres = fcopy_r(path2, path);
	if (fres != FR_OK) {
		printf(pBad "fcopy_r failed! (%i)\n", fres);
		return false;
	}

	sprintf(strrchr(path, '/'),  "/%08x", mset9.region->MiiMakerExtData);
	sprintf(strrchr(path2, '/'), "/%08x", mset9.region->MiiMakerExtData);

	fres = fcopy_r(path2, path);
	if (fres != FR_OK) {
		printf(pBad "fcopy_r failed! (%i)\n", fres);
		return false;
	}

	// 3. Copy databases
	puts(pInfo "Copying databases over...");
	strcpy(strchr(path, '/'),  "/dbs");
	strcpy(strchr(path2, '/'), "/dbs");

	fres = fcopy_r(path2, path);
	if (fres != FR_OK) {
		printf(pBad "fcopy_r failed! (%i)\n", fres);
		return false;
	}

	// 4. Create trigger file (002F003A.txt)
	puts(pInfo "Injecting trigger file...");
	strcpy(strchr(path, '/'), "/extdata/002F003A.txt");
	fres = f_open(&fp, path, FA_CREATE_NEW | FA_WRITE);
	if (fres != FR_OK) {
		printf(pBad "f_open failed! (%i)\n", fres);
		return false;
	}

	UINT written;
	fres = f_write(&fp, "arm9 \"security\" processor vs. text file in extdata:", 52, &written);
	f_close(&fp);

	// 5. Back up original ID1
	puts(pInfo "Backing up original ID1...");
	strcpy(path,  mset9.ID[1]);
	strcpy(path2, mset9.ID[1]);
	strcat(path2, ID1backupTag);

	fres = f_rename(path, path2);
	if (fres != FR_OK) {
		printf(pBad "f_rename failed! (%i)\n", fres);
		return false;
	}

	puts(pGood "MSET9 injected, have fun!");
	return true;
}
