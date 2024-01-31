#include <stdio.h>
#include <stdint.h>

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
	int consoleVer, consoleRegion;
	char ID0[32 +1], ID1[32 +1];
};

static struct MSET9 mset9 = {};

static bool is3DSID(const char* name) {
	if (strlen(name) != 32) return false;

	// fuck it
	uint32_t idparts[4];
	if (sscanf(name, "%08x%08x%08x%08x", idparts++, idparts++, idparts++, idparts++) != 4) // This is not an ID0/ID1
		return false;

}

bool MSET9Start(int consoleVer) {
	static char path[PATH_MAX];
	struct dirent* pent = NULL;
	DIR* pdir = NULL;

	strcpy(path, "/Nintendo 3DS");
	pdir = opendir(path);
	if (!pdir) {
		puts(pBad "Error #01: Nintendo 3DS folder not found! Is your console reading the SD card?");
		return false;
	}

	bool foundID0 = false;
	while ((pent = readdir(pdir))) {
		if (strcasecmp(pent->d_name, "Private") == 0) continue;

		if (is3DSID(pent->d_name)) {
			if (foundID0) {
				puts(pBad  "Error #07: You have multiple ID0's!");
				puts(pInfo "Consult: https://wiki.hacks.guide/wiki/3DS:MID0");
				return false;
			}
			foundID0 = true;
			strcpy(mset9.ID0, pent->d_name);
		}
	}
	closedir(pdir);

	strcat(path, "/");
	strcat(path, mset9.ID0);
	pdir = opendir(path);
	if (!pdir) {
		perror(pBad "Failed to open ID0 folder (!?)");
		return false;
	}

	bool foundId1 = false;
	while ((pent = readdir(pdir))) {
		if (is3DSID(pent->d_name)) {
			if (foundID1) {
				puts(pBad  "Error #12: You have multiple ID1's!");
				puts(pInfo "Consult: https://wiki.hacks.guide/wiki/3DS:MID1");
				return false;
			}
			foundID1 = true;
			strcpy(mset9.ID1, pent->d_name);
		}
	}
	closedir(pdir);


}

bool MSET9SanityCheck(void) {
	puts(pInfo "Performing sanity checks...");
	if (!CheckFile("/boot9strap/boot9strap.firm", 0, true)
	||	!CheckFile("/boot.firm", 0, false)
	||	!CheckFile("/boot.3dsx", 0, false)
	||	!CheckFile("/b9", 0, false)
	||	!CheckFile("/SafeB9S.bin", 0, false))
	{
		puts(pBad  "Error #08: One or more files are missing!");
		puts(pInfo "Please re-extract the MSET9 zip file, overwriting any files when prompted.");

		return false;
	}


}
