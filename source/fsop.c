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
		printf(pBad "	%ls: Does not exist on SD card!\n", filepath);
		return false;
	}
	else if (fres != FR_OK) {
		printf(pBad "	%ls: f_stat() failed (%i)\n", filepath, fres);
		return false;		
	}

	if (st.fattrib & AM_DIR) {
		printf(pBad "	%ls: Is a directory\n", filepath);
		return false;
	}

	if (filesize && st.fsize != filesize) {
		float mb_filesize = filesize / 1048576.f, mb_stfilesize = st.fsize / 1048576.f;
		printf(pBad "	%ls: File size incorrect! (expected %.2fMB, is %.2fMB)\n", filepath, mb_filesize, mb_stfilesize);
		return false;
	}

	if (verifySHA256) {
		VRESULT vr = VerifyHash(filepath);
		if (vr != VR_OK) {
			printf(pBad "	%ls: SHA256 verification failed! (%i)\n", filepath, vr);
			return false;
		}	
	}

	printf(pGood "	%ls: File OK! %s\n", filepath, verifySHA256? "(Hash verified)" : "");
	return true;
}

VRESULT VerifyHash(const wchar_t* filepath) {
	static unsigned char buffer[0x1000];
	wchar_t path[0x80] = {};
	FIL fp = {}, fp_sha = {};
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
	fres = f_open(&fp_sha, path, FA_READ);
	if (fres != FR_OK) return VR_NO_HASHFILE;

	unsigned char hash[SHA256_BLOCK_SIZE] = {};
	UINT read = 0;
	fres = f_read(&fp_sha, hash, sizeof(hash), &read);
	f_close(&fp_sha);

	if (read != sizeof(hash)) return VR_NO_HASHFILE;
	else if (memcmp(hashCalc, hash, sizeof(hash)) != 0) return VR_MISMATCH;

	return VR_OK;
}
