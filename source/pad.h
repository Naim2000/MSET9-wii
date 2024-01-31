#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#include <gctypes.h>

void initpads();
void scanpads();
void wait_button(u32);
u32 buttons_down(u32);
