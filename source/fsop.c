#include <stdio.h>
#include <string.h>
#include <ogc/machine/processor.h>
#include <errno.h>

#include "video.h"
#include "fsop.h"
#include "sha256.h"
#include "firm.h"

static unsigned char buffer[0x4000] __aligned(0x40);

bool CheckFile(const TCHAR* filepath, size_t filesize, bool verifySHA256) {
	FRESULT fres = 0;
	FILINFO st = {};
	FIL fp = {};

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

	fres = f_open(&fp, filepath, FA_READ | (FA_OPEN_APPEND & ~FA_OPEN_ALWAYS));
	f_close(&fp);
	if (fres != FR_OK) {
		printf(pBad "	%s: Failed to open file! Is the file broken?\n", filepath);
		return false;
	}

	// Should at least have a size
	if (!st.fsize) {
		printf(pBad "	%s: File has no size! Was it closed properly?\n", filepath);
		return false;
	}

	else if (filesize && st.fsize != filesize) {
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

	if (strrchr(filepath, '.') && !strcmp(strrchr(filepath, '.'), ".firm")) {
		int res = VerifyFIRM(filepath);
		if (res) {
			printf(pBad "	%s: FIRM verification failed! (%i)\n", filepath, res);
			return false;
		}
	}

	printf(pGood "	%s: File OK!\n", filepath);
	return true;
}

static SHA256_CTX sha256ctx = {};
static UINT sha256hash_cb(const BYTE* data, UINT size) {
	if (!size) return true;

	sha256_update(&sha256ctx, data, size);
	return size;
}

static FRESULT hashfile(FIL* fp, FSIZE_t offset, UINT size, unsigned char out[SHA256_BLOCK_SIZE]) {
	FRESULT fres;
	UINT forwarded;

	fres = f_lseek(fp, offset);
	if (fres) return fres;

	sha256_init(&sha256ctx);
	fres = f_forward(fp, sha256hash_cb, size, &forwarded);
	sha256_final(&sha256ctx, out);

	return fres;
}

int VerifyFIRM(const TCHAR* filepath) {
	int res;
	FIL fp[1];
	FIRMHeader header[1];

	res = f_open(fp, filepath, FA_READ);
	if (res) return res;

	UINT read;
	res = f_read(fp, header, sizeof(FIRMHeader), &read);
	if (read != sizeof(FIRMHeader)) {
		f_close(fp);
		if (!res) res = ~32;
		return res;
	}

	if (memcmp(header->magic, "FIRM", 4) != 0) {
		f_close(fp);
		return ~7;
	}

	for (FIRMSection* sect = header->sections; sect < header->sections + 4; sect++) {
		unsigned char hash[SHA256_BLOCK_SIZE] = {};
		uint32_t size = __lwbrx(sect, offsetof(FIRMSection, size));
		uint32_t offset = __lwbrx(sect, offsetof(FIRMSection, offset));

		if (!size) continue;

		res = hashfile(fp, offset, size, hash);
		if (res) break;

		if (memcmp(sect->hash, hash, SHA256_BLOCK_SIZE) != 0) {
			res = ~48;
			break;
		}
	}
	f_close(fp);

	return res;
}

VRESULT VerifyHash(const TCHAR* filepath) {
	TCHAR path[160];
	FRESULT fres = 0;
	FIL fp = {}, fp_sha = {};
	unsigned char hashA[SHA256_BLOCK_SIZE] = {};
	unsigned char hashB[SHA256_BLOCK_SIZE] = {};

	fres = f_open(&fp, filepath, FA_READ);
	if (fres != FR_OK) return VR_NO_FILE;

	hashfile(&fp, 0, -1, hashA);
	f_close(&fp);

	sprintf(path, "%s.sha", filepath);
	fres = f_open(&fp_sha, path, FA_READ);
	if (fres != FR_OK) return VR_NO_HASHFILE;

	UINT read = 0;
	fres = f_read(&fp_sha, hashB, sizeof(hashB), &read);
	f_close(&fp_sha);

	if (read != sizeof(hashB)) return VR_NO_HASHFILE;
	else if (memcmp(hashA, hashB, sizeof(hashB)) != 0) return VR_MISMATCH;

	return VR_OK;
}

FRESULT f_rmdir_r(const TCHAR* path) {
	TCHAR path_b[384];

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
	TCHAR src_b[384], dst_b[384];

	FRESULT fres;
	FIL fp[2] = {};
	DIR dp = {};
	FILINFO st[1] = {};

//	puts(dst);

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

		fres = f_open(&fp[1], dst_b, FA_WRITE | FA_CREATE_NEW);
		if (fres != FR_OK) goto close;

		UINT read, written, total = 0;
		while (true)
		{
			fres = f_read(&fp[0], buffer, sizeof(buffer), &read);
			if (fres || !read) break;

			fres = f_write(&fp[1], buffer, read, &written);
			total += written;
			if (fres || written != read) break;
		}

// i don't love executable stack sorry
#if 0
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
#endif

	close:
		f_close(&fp[0]);
		f_close(&fp[1]);
		if (fres == FR_OK && total != st->fsize) fres = FR_DENIED;
		if (fres != FR_OK) break;
	}
	f_closedir(&dp);

	return fres;
}

FRESULT f_dummy(const TCHAR* path, bool force) {
	FIL fp;

	FRESULT fres = f_open(&fp, path, FA_WRITE | (force ? FA_CREATE_ALWAYS : FA_CREATE_NEW));
	f_close(&fp);
	return fres;
}
