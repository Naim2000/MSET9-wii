#include <wchar.h>

#define CONSOLE_HEIGHT		(480-32)
#define CONSOLE_WIDTH		(640)

#define prbad(fmt, ...)			printf("[\x1b[31;1mXX\x1b[39m] " fmt "\n", ##__VA_ARGS__)
#define prgood(fmt, ...)		printf("[\x1b[32;1mOK\x1b[39m] " fmt "\n", ##__VA_ARGS__)
#define prwarn(fmt, ...)		printf("[\x1b[33;1m??\x1b[39m] " fmt "\n", ##__VA_ARGS__)
#define prnote(fmt, ...)		printf("[\x1b[34;1m**\x1b[39m] " fmt "\n", ##__VA_ARGS__)
#define prinfo(fmt, ...)		printf("[--] " fmt "\n", ##__VA_ARGS__)

extern int conX, conY;

void init_video();
void clear();
void clearln(char c);
