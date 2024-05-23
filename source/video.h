#include <wchar.h>

#define CONSOLE_HEIGHT		(480-32)
#define CONSOLE_WIDTH		(640)

#define pBad	"[\x1b[31;1mXX\x1b[39m]	"
#define pGood	"[\x1b[32;1mOK\x1b[39m]	"
#define pWarn	"[\x1b[33;1m!?\x1b[39m]	"
#define pInfo	"[--]	"
#define pNote	"[\x1b[34;1m**\x1b[39m]	"

void init_video();
void clear();
void clearln();
