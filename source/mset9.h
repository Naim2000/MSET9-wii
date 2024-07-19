#include <stdbool.h>

typedef enum MSET9Version {
	OLD_11_8__11_17, 
	NEW_11_8__11_17, 
	OLD_11_4__11_7,
	NEW_11_4__11_7
} MSET9Version;

struct MSET9 {
	MSET9Version consoleVer;
	char ID[2][32 +1];
	bool setup: 1;
	bool hasBackupID1: 1;
	bool hasHaxID1: 1;
	bool titleDbsOK: 1;
	bool homeExtdataOK: 1;
	bool miiExtdataOK: 1;
	bool hasTriggerFile: 1;
};

extern struct MSET9 mset9;

// Identify the user's ID0 and ID1, hacked ID1 if present
int MSET9Start(void);

// Create the hacked ID1
bool MSET9CreateHaxID1(MSET9Version);

// Check global exploit files
bool MSET9SanityCheckA(void);

// Check console specific exploit files
bool MSET9SanityCheckB(void);

// (Un)inject MSET9.
bool MSET9Injection(bool create);

// Fully remove MSET9.
void MSET9Remove(void);
