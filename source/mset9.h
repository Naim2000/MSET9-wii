#include <stdbool.h>

// Identify the user's ID0 and ID1.
bool MSET9Start(void);

enum {
	O3DS_11_8_11_17,
	N3DS_11_8_11_17,
	O3DS_11_4_11_7,
	N3DS_11_4_11_7,
};

// This should check if the user has selected the correct console version. (If mset9 is already present)
// bool MSET9SetConsoleRev(int);

// Check extracted files, HOME menu/Mii maker extdata, etc.
bool MSET9SanityCheck(void);
