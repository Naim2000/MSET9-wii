#include <wchar.h>

#define CONSOLE_HEIGHT		(480-32)
#define CONSOLE_WIDTH		(640)

#define pBad	"[\x1b[31;1m\a\x1b[39m]	"
#define pGood	"[\x1b[32;1m\a\x1b[39m]	"
#define pWarn	"[\x1b[33;1m\a\x1b[39m]	"
#define pInfo	"[\a]	"

void init_video();
void clear();
void clearln();
