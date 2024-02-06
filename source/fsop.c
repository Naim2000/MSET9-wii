#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>

#include "video.h"
#include "fsop.h"
#include "sha256.h"

bool CheckFile(const wchar_t* filepath, size_t filesize, bool verifySHA256) {
	FRESULT fres = 0;
	static FILINFO st = {};

	fres = f_stat(filepath, &st);
	if (fres == FR_NO_FILE || fres == FR_NO_PATH) {
		wprintf(pBad "	%ls: Does not exist on SD card!\n", filepath);
		return false;
	}
	else if (fres != FR_OK) {
		wprintf(pBad "	%ls: f_stat() failed (%i)\n", filepath, fres);
		return false;		
	}

	if (st.fattrib & AM_DIR) {
		wprintf(pBad "	%ls: Is a directory\n", filepath);
		return false;
	}

	if (filesize && st.fsize != filesize) {
		float kb_filesize = filesize / 1024.f, kb_stfilesize = st.fsize / 1024.f;
		wprintf(pBad "	%ls: File size incorrect! (expect %.2fKB, has %.2fKB)\n", filepath, kb_filesize, kb_stfilesize);
		return false;
	}

	if (verifySHA256) {
		VRESULT vr = VerifyHash(filepath);
		if (vr != VR_OK) {
			wprintf(pBad "	%ls: SHA256 verification failed! (%i)\n", filepath, vr);
			return false;
		}	
	}

	wprintf(pGood "	%ls: File OK! %s\n", filepath, verifySHA256? "(Hash verified)" : "");
	return true;
}

VRESULT VerifyHash(const wchar_t* filepath) {
	static unsigned char buffer[0x1000];
	static wchar_t path[0x400];
	FIL fp = {};
	FRESULT fres = 0;

	wcscpy(path, filepath);

	fres = f_open(&fp, path, FA_READ);
	if (fres != FR_OK) return VR_NO_FILE;

	unsigned char hashCalc[SHA256_BLOCK_SIZE] = {};
	SHA256_CTX sha = {};
	sha256_init(&sha);

	while (true) {
		UINT read = 0;
		fres = f_read(&fp, buffer, sizeof(buffer), &read);
		if (fres != FR_OK || !read) break;
		sha256_update(&sha, buffer, read);
	}

	sha256_final(&sha, hashCalc);
	f_close(&fp);

	wcscat(path, L".sha");
	fres = f_open(&fp, path, FA_READ);
	if (fres != FR_OK) return VR_NO_HASHFILE;

	unsigned char hash[SHA256_BLOCK_SIZE] = {};
	UINT read = 0;
	fres = f_read(&fp, hash, sizeof(hash), &read);
	f_close(&fp);

	if (read != sizeof(hash)) return VR_NO_HASHFILE;
	else if (memcmp(hashCalc, hash, sizeof(hash)) != 0) return VR_MISMATCH;

	return VR_OK;
}
