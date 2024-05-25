#include <stdbool.h>

#include "fatfs/ff.h"

typedef enum {
	VR_OK,
	VR_NO_FILE,
	VR_NO_HASHFILE,
	VR_MISMATCH,

} VRESULT;

bool CheckFile(const char*, size_t, bool VerifySHA256);
int VerifyFIRM(const char*);
VRESULT VerifyHash(const char*);

FRESULT f_rmdir_r(const TCHAR* path);
FRESULT fcopy_r(const TCHAR* src, const TCHAR* dst);
