/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

void display_set(display_f);
void display_load(void);
draw_f display_setDrawFunc(draw_f);
void display_update(void);

typedef enum
{
	CRTANIM_CLOSE,
	CRTANIM_OPEN
} crtAnim_t;

void display_startCRTAnim(crtAnim_t);
bool display_is_ani_active(void);

#endif /* DISPLAY_H_ */
