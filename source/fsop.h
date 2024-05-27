#include <stdbool.h>

#include "fatfs/ff.h"

typedef enum {
	VR_OK,
	VR_NO_FILE,
	VR_NO_HASHFILE,
	VR_MISMATCH,

} VRESULT;

bool CheckFile(const TCHAR*, size_t, bool VerifySHA256);
int VerifyFIRM(const TCHAR*);
VRESULT VerifyHash(const TCHAR*);

FRESULT f_rmdir_r(const TCHAR* path);
FRESULT fcopy_r(const TCHAR* src, const TCHAR* dst);
FRESULT f_dummy(const TCHAR* path, bool force);
