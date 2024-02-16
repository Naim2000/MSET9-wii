#include <gccore.h>
#include <gctypes.h>
#include <wiiuse/wpad.h>

#include "pad.h"

static uint32_t pad_buttons;

void initpads() {
	WPAD_Init();
}

void scanpads() {
	WPAD_ScanPads();
	pad_buttons = WPAD_ButtonsDown(0);

	if (SYS_ResetButtonDown()) pad_buttons |= WPAD_BUTTON_HOME;
}

void wait_button(uint32_t button) {
	scanpads();
	while (!(pad_buttons & (button? button : ~0)) )
		scanpads();
}

uint32_t buttons_down(uint32_t button) {
	return pad_buttons & (button? button : ~0);
}



