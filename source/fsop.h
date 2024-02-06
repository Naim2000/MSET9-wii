#include <stdbool.h>

#include "fatfs/ff.h"

typedef enum {
	VR_OK,
	VR_NO_FILE,
	VR_NO_HASHFILE,
	VR_MISMATCH,

} VRESULT;

bool CheckFile(const wchar_t*, size_t, bool verifySHA256);
VRESULT VerifyHash(const wchar_t*);
