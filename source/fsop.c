#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "video.h"
#include "fsop.h"
#include "sha256.h"

bool CheckFile(const char* filepath, size_t filesize, bool verifySHA256) {
	static struct stat st = {};
	if (stat(filepath, &st) < 0) {
		printf(pBad "%s: %s\n", filepath, strerror(errno));
		return false;
	}

	if (filesize && st.st_size != filesize) {
		float kb_filesize = filesize / 1024.f, kb_stfilesize = st.st_size / 1024.f;
		printf(pBad "%s: File size incorrect! (expect %.2fKB, has %.2fKB)\n", filepath, kb_filesize, kb_stfilesize);
		return false;
	}

	if (verifySHA256 && !VerifyHash(filepath)) {
		printf(pBad "%s: SHA256 checksum incorrect! File is corrupt!\n", filepath);
		return false;
	}

	printf(pGood "%s: File OK!\n", filepath);
	return true;
}

bool VerifyHash(const char* filepath) {
	static unsigned char buffer[0x1000];
	static char path[PATH_MAX];
	strcpy(path, filepath);

	FILE* fp = fopen(path, "rb");
	if (!fp) return false;

	SHA256_CTX sha = {};
	unsigned char hashCalc[SHA256_BLOCK_SIZE] = {};
	sha256_init(&sha);

	while (true) {
		size_t read = fread(buffer, 1, sizeof(buffer), fp);
		if (!read) break;
		sha256_update(&sha, buffer, read);
	}
	sha256_final(&sha, hashCalc);
	fclose(fp);

	strcat(path, ".sha");
	FILE* hashFile = fopen(path, "rb");
	if (!hashFile) return false;

	unsigned char hash[SHA256_BLOCK_SIZE] = {};
	bool read = fread(hash, 1, sizeof(hash), hashFile) == sizeof(hash);
	fclose(hashFile);

	return read && !memcmp(hashCalc, hash, sizeof(hash));
}
