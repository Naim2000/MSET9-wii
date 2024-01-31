#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

#include "video.h"
#include "pad.h"
#include "sd.h"

int main(void) {
	printf("meme set 9\n\n");

	initpads();
	if (!SDMount()) goto exit;



exit:
	SDUnmount();
	puts("Press HOME to exit.");
	wait_button(WPAD_BUTTON_HOME);
	return 0;
}
