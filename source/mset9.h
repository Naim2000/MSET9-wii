#include <stdbool.h>

typedef enum MSET9Version {
	O3DS_11_8_11_17 = 1,
	N3DS_11_8_11_17 = 2,
	O3DS_11_4_11_7  = 3,
	N3DS_11_4_11_7  = 4,
} MSET9Version;

// Identify the user's ID0 and ID1.
bool MSET9Start(void);

// This should check if the user has selected the correct console version. (If mset9 is already present)
bool MSET9SetConsoleVer(MSET9Version);

// Check extracted files, HOME menu/Mii maker extdata, etc.
bool MSET9SanityCheck(void);

// Inject MSET9.
bool MSET9Injection(void);
