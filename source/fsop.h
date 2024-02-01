#include <stdbool.h>
#include <sys/stat.h>
#include <dirent.h>

bool CheckFile(const char*, size_t, bool verifySHA256);
bool VerifyHash(const char*);
