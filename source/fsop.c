#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>

#include "video.h"
#include "fsop.h"
#include "sha256.h"

bool CheckFile(const char* filepath, size_t filesize, bool verifySHA256) {
	FRESULT fres = 0;
	static FILINFO st = {};

	fres = f_stat(filepath, &st);
	if (fres == FR_NO_FILE || fres == FR_NO_PATH) {
		printf(pBad "	%s: Does not exist on SD card!\n", filepath);
		return false;
	}
	else if (fres != FR_OK) {
		printf(pBad "	%s: f_stat() failed (%i)\n", filepath, fres);
		return false;		
	}

	if (st.fattrib & AM_DIR) {
		printf(pBad "	%s: Is a directory\n", filepath);
		return false;
	}

	// Should at least have a size
	if (!st.fsize) {
		printf(pBad "	%s: File has no size! Was it closed properly?\n", filepath);
		return false;
	}

	if (filesize && st.fsize != filesize) {
		float mb_filesize = filesize / 1048576.f, mb_stfilesize = st.fsize / 1048576.f;
		printf(pBad "	%s: File size incorrect! (expected %.2fMB, is %.2fMB)\n", filepath, mb_filesize, mb_stfilesize);
		return false;
	}

	if (verifySHA256) {
		VRESULT vr = VerifyHash(filepath);
		if (vr != VR_OK) {
			printf(pBad "	%s: SHA256 verification failed! (%i)\n", filepath, vr);
			return false;
		}	
	}

	printf(pGood "	%s: File OK! %s\n", filepath, verifySHA256? "(Hash verified)" : "");
	return true;
}

VRESULT VerifyHash(const char* filepath) {
	static unsigned char buffer[0x1000];
	char path[100];
	FIL fp = {}, fp_sha = {};
	FRESULT fres = 0;

	fres = f_open(&fp, filepath, FA_READ);
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

	sprintf(path, "%s.sha", filepath);
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

FRESULT f_rmdir_r(const TCHAR* path) {
	TCHAR path_b[256];

	FRESULT fres;
	DIR dp = {};
	FILINFO st[1] = {};

	fres = f_opendir(&dp, path);
	if (fres != FR_OK) return fres;

	while ((fres = f_readdir(&dp, st)) == FR_OK && st->fname[0]) {
		sprintf(path_b, "%s/%s", path, st->fname);

		if (st->fattrib & AM_DIR)
			fres = f_rmdir_r(path_b);
		else
			fres = f_unlink(path_b);

		if (fres != FR_OK) break;
	}
	f_closedir(&dp);
	fres = f_unlink(path);

	return fres;
}

FRESULT fcopy_r(const TCHAR* src, const TCHAR* dst) {
	TCHAR src_b[256], dst_b[256];

	FRESULT fres;
	FIL fp[2] = {};
	DIR dp = {};
	FILINFO st[1] = {};

	puts(dst);

	fres = f_mkdir(dst);
	if (fres != FR_OK && fres != FR_EXIST) return fres;

	fres = f_opendir(&dp, src);
	if (fres != FR_OK) return fres;

	while ((fres = f_readdir(&dp, st)) == FR_OK && st->fname[0]) {
		sprintf(src_b, "%s/%s", src, st->fname);
		sprintf(dst_b, "%s/%s", dst, st->fname);

		if (st->fattrib & AM_DIR) {
			fres = fcopy_r(src_b, dst_b);
			if (fres != FR_OK) break;
			continue;
		}

		// static unsigned char buf[0x1000];

		fres = f_open(&fp[0], src_b, FA_READ);
		if (fres != FR_OK) break;

		fres = f_open(&fp[1], dst_b, FA_WRITE);
		if (fres != FR_OK) goto close;

		// i love executable stack
		UINT forward_cb(const BYTE* data, UINT len)
		{
			if (!len) return true; // Ready

			UINT written;

			f_write(&fp[1], data, len, &written);
			return written;
		}

		UINT written;
		fres = f_forward(&fp[0], forward_cb, st->fsize, &written);

	close:
		f_close(&fp[0]);
		f_close(&fp[1]);
		if (fres == FR_OK && written != st->fsize) fres = FR_DENIED;
		if (fres != FR_OK) break;
	}
	f_closedir(&dp);

	return fres;
}
