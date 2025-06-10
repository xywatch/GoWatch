#include "common.h"
extern bool keep_on;
extern float BatteryVol;

static display_t draw() {
		char buf[49];
	
    u8 y = 0;
    draw_string(PSTR("User: " USER_NAME), false, 0, y);
    draw_string(PSTR("FW: " FW_VERSION), false, 0, y += 8);
		sprintf(buf, "HW: %d", HW_VERSION);
    draw_string(buf, false, 0, y += 8);
    // draw_fill_screen(0, y, 127, y, 1);
    // y += 2;
    
    sprintf(buf, "Voltage: %0.2f", BatteryVol);
    draw_string(buf, false, 0, y += 8);

    return DISPLAY_DONE;
}

static bool btnExit()
{
  keep_on = false;
  back_to_watchface();
  return true;
}

void showAbout(void)
{
  display_setDrawFunc(draw);
  buttons_setFuncs(btnExit, btnExit, btnExit);
  showMeThenRun(NULL);
  keep_on = true;
}