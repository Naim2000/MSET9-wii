#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>

#include "mset9.h"
#include "video.h"
#include "fsop.h"

#define ID1backupTag L"_user-id1"

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

struct N3DSRegion
{
	char name[4];
	uint32_t homeMenuExtData;
	uint32_t MiiMakerExtData;
};

static const struct N3DSRegion N3DSRegions[NBR_REGIONS] = {
	{ "USA", 0x8F, 0x217 },
	{ "EUR", 0x98, 0x227 },
	{ "JPN", 0x82, 0x207 },
	{ "CHN", 0xA1, 0x267 },
	{ "KOR", 0xA9, 0x277 },
	{ "TWN", 0xB1, 0x287 },
};

/*
 * o3DS 11.8<->11.17: "FFFFFFFA119907488546696508A10122054B984768465946C0AA171C4346034CA047B84700900A0871A0050899CE0408730064006D00630000900A0862003900",
 * n3DS 11.8<->11.17: "FFFFFFFA119907488546696508A10122054B984768465946C0AA171C4346034CA047B84700900A0871A005085DCE0408730064006D00630000900A0862003900",
 * o3DS 11.4<->11.7 : "FFFFFFFA119907488546696508A10122054B984768465946C0AA171C4346034CA047B84700900A08499E050899CC0408730064006D00630000900A0862003900",
 * n3DS 11.4<->11.7 : "FFFFFFFA119907488546696508A10122054B984768465946C0AA171C4346034CA047B84700900A08459E050881CC0408730064006D00630000900A0862003900"
 */

struct MSET9 {
	int consoleVer;
	const struct N3DSRegion* region;
	wchar_t ID0[32 +1], ID1[32 +1];
};

static struct MSET9 mset9 = {};

static bool is3DSID(const wchar_t* name) {
	if (wcslen(name) != 32) return false;

	// fuck it
	uint32_t _idparts[4];
	uint32_t *idparts = _idparts;

	return (swscanf(name, L"%08x%08x%08x%08x", idparts++, idparts++, idparts++, idparts++) == 4);
}

bool MSET9Start(void) {
	static wchar_t path[0x400];
	FRESULT fres = 0;
	DIR dp = {};
	FILINFO fl = {};

	wcscpy(path, L"/Nintendo 3DS");
	fres = f_opendir(&dp, path);
	if (fres == FR_NO_PATH) {
		wprintf(pBad "Error #01: Nintendo 3DS folder not found!\n"
				pBad "Is your console reading the SD card?\n");
		return false;
	}
	else if (fres != FR_OK) {
		wprintf(pBad "Failed to open Nintendo 3DS folder (!?) (%i)\n", fres);
		return false;
	}

	bool foundID0 = false;
	while (true) {
		fres = f_readdir(&dp, &fl);
		if (fres != FR_OK || !fl.fname[0]) break;

		if (is3DSID(fl.fname)) {
			if (foundID0) {
				wprintf(pBad "Error #07: You have multiple ID0's!\n"
						pInfo "Consult: https://wiki.hacks.guide/wiki/3DS:MID0\n");
				return false;
			}
			foundID0 = true;
			wcscpy(mset9.ID0, fl.fname);
		}
	}
	f_closedir(&dp);

	if (!foundID0) {
		wprintf(pBad "Error #07: You have...... no ID0?\n"
				pBad "Is your console reading the SD card?\n");
		return false;
	}

	wcscat(path, L"/");
	wcscat(path, mset9.ID0);
	fres = f_opendir(&dp, path);
	if (fres != FR_OK) {
		wprintf(pBad "Failed to open ID0 folder (!?) (%i)\n", fres);
		return false;
	}

	bool foundID1 = false;
	while (true) {
		fres = f_readdir(&dp, &fl);
		if (fres != FR_OK || !fl.fname[0]) break;

		if (is3DSID(fl.fname)) {
			if (foundID1) {
				wprintf(pBad "Error #12: You have multiple ID1's!\n"
						pInfo "Consult: https://wiki.hacks.guide/wiki/3DS:MID1\n");
				return false;
			}
			foundID1 = true;
			wcscpy(mset9.ID1, fl.fname);
		}
	}
	f_closedir(&dp);

	if (!foundID1) {
		wprintf(pBad  "Error #12: You have...... no ID1?\n"
				pBad "Is your console reading the SD card?\n");
		return false;
	}

	return true;
}

bool MSET9SanityCheck(void) {
	static wchar_t path[0x400];
	wprintf(pInfo "Performing sanity checks...\n");

	wprintf(pInfo "Checking extracted files...\n");
	if (!CheckFile(L"/boot9strap/boot9strap.firm", 0, true)
	||	!CheckFile(L"/boot.firm", 0, false)
	||	!CheckFile(L"/boot.3dsx", 0, false)
	||	!CheckFile(L"/b9", 0, false)
	||	!CheckFile(L"/SafeB9S.bin", 0, false))
	{
		wprintf(pBad  "Error #08: One or more files are missing or malformed!\n");
		wprintf(pInfo "Please re-extract the MSET9 zip file, overwriting any files when prompted.\n");

		return false;
	}
	wprintf(pGood "Extracted files look good!\n");

	swprintf(path, sizeof(path), L"/Nintendo 3DS/%.32s/%.32s/", mset9.ID0, mset9.ID1);
	f_chdir(path);

	wprintf(pInfo "Checking databases...\n");
	if (!CheckFile(L"dbs/import.db", dbsSize, false) || !CheckFile(L"dbs/title.db", dbsSize, false)) {
		wprintf(pBad  "Error #10: Databases malformed/missing!\n\n");

		FIL db = {};
		f_mkdir(L"dbs");
		if (f_open(&db, L"dbs/import.db", FA_CREATE_NEW | FA_WRITE) == FR_OK) f_close(&db);
		if (f_open(&db, L"dbs/title.db", FA_CREATE_NEW | FA_WRITE) == FR_OK) f_close(&db);

		wprintf(pInfo "Please reset the title database by navigating to\n"
				pInfo "	System Settings -> Data Management -> Nintendo 3DS\n"
				pInfo "	-> Software -> Reset, then re-run the installer.\n\n"

				pInfo "Visual aid: https://3ds.hacks.guide/images/screenshots/database-reset.jpg\n"
		);

		return false;
	}
	wprintf(pGood "Databases look good!\n");

	wprintf(pInfo "Looking for HOME menu extdata...\n");
	wchar_t extDataPath[30];

	for (int i = 0; i < NBR_REGIONS; i++) {
		const struct N3DSRegion* rgn = N3DSRegions + i;

		swprintf(extDataPath, sizeof(extDataPath), L"extdata/00000000/%08x", rgn->homeMenuExtData);
		if (f_stat(extDataPath, 0) == FR_OK) {
			wprintf(pGood "Found %s HOME menu extdata!\n", rgn->name);
			mset9.region = rgn;
			break;
		}
	}

	if (!mset9.region) {
		wprintf(pBad "Error #04: No HOME menu extdata found!\n"
				pBad "Is your console reading your SD card?\n");
		return false;
	}

	wprintf(pInfo "Looking for Mii Maker extdata...\n");
	swprintf(extDataPath, sizeof(extDataPath), L"extdata/00000000/%08x", mset9.region->MiiMakerExtData);
	if (f_stat(extDataPath, 0) != FR_OK) {
		wprintf(pBad "Error #05: No Mii Maker extdata found!\n");
		return false;
	}
	wprintf(pGood "Found Mii Maker extdata!\n");

	return true;
}
