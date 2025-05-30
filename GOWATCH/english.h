/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2014 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#ifndef ENGLISH_H_
#define ENGLISH_H_

// String buffer sizes
// Don't forget to add 1 for null terminator
#define BUFFSIZE_STR_MENU 24
#define BUFFSIZE_STR_DAYS 4
#define BUFFSIZE_STR_MONTHS 4

#define BUFFSIZE_DATE_FORMAT ((BUFFSIZE_STR_DAYS - 1) + (BUFFSIZE_STR_MONTHS - 1) + 12)
#define BUFFSIZE_TIME_FORMAT_SMALL 9

// String formats
#define DATE_FORMAT ("%s %02hhu %s 20%02hhu")
#define TIME_FORMAT_SMALL ("%02hhu:%02hhu%c")

#define CHAR_DEGREES 127
#define CHAR_TICK 128

#define CHAR_AM 'A'
#define CHAR_PM 'P'
#define CHAR_24 ' '

#define CHAR_YES 'Y'
#define CHAR_NO 'N'

#define STR_DOWCHARS "MTWTFSS"

// Days
// Also see BUFFSIZE_STR_DAYS
#define STR_MON "Mon"
#define STR_TUE "Tue"
#define STR_WED "Wed"
#define STR_THU "Thu"
#define STR_FRI "Fri"
#define STR_SAT "Sat"
#define STR_SUN "Sun"

// Months
// Also see BUFFSIZE_STR_MONTHS
#define STR_JAN "Jan"
#define STR_FEB "Feb"
#define STR_MAR "Mar"
#define STR_APR "Apr"
#define STR_MAY "May"
#define STR_JUN "Jun"
#define STR_JUL "Jul"
#define STR_AUG "Aug"
#define STR_SEP "Sep"
#define STR_OCT "Oct"
#define STR_NOV "Nov"
#define STR_DEC "Dec"

// Menu strings
// Also see BUFFSIZE_STR_MENU

#define STR_MAINMENU "< MAIN MENU >"
#define STR_ALARMS "Alarms"
#define STR_FLASHLIGHT "Flashlight"
#define STR_STOPWATCH "Stopwatch"
#define STR_GAMES "Games"
#define STR_SETTINGS "Settings"
#define STR_DIAGNOSTICS "Diagnostics"
// #define STR_BTRCCAR		"BT RC Car"
#define STR_TUNEMAKER "Tune maker"
// #define STR_CALCULATORS	"Calculators"
#define STR_CALENDAR "Calendar"

#define STR_ALARMSMENU "< ALARMS >"

#define STR_TIMEDATEMENU "< TIME & DATE >"
#define STR_SAVE "Save"
#define STR_SAVED "Saved"

#define STR_DIAGNOSTICSMENU "< DIAGNOSTICS >"
// #define STR_TEMPERATURE		"Temperature %hhd.%hhuC"
// #define STR_BATTERY			"Battery    %umV"
// #define STR_SHOWFPS			"Show FPS%9c"

#define STR_DISPLAYMENU "< DISPLAY >"
#define STR_BRIGHTNESS "Brightness"
#define STR_INVERT "Invert"
#define STR_ROTATE "Rotate"
#define STR_ANIMATIONS "Animations"
#define STR_SETFPS "FPS"

#define STR_GAMESMENU "< GAMES >"

#define STR_SOUNDMENU "< SOUND >"
#define STR_UI "UI"
#define STR_HOURBEEPS "Hour beeps"

#define STR_SLEEPMENU "< SLEEP >"
#define STR_TIMEOUT "Timeout"
#define STR_MOVECHECK "MoveCheck"
// #define STR_CLOCKMODE	"Clock mode"

#define STR_SETTINGSMENU "< SETTINGS >"
#define STR_TIMEDATE "Time & date"
#define STR_SLEEP "Sleep"
#define STR_SOUND "Sound"
#define STR_DISPLAY "Display"
#define STR_LEDS "LEDs"
#define STR_RCSETTINGS "RC Settings"

#define STR_BACK "Back"
#define STR_EXIT "Exit"

// Game strings

#define STR_GAME1 "Breakout"
#define STR_GAME2 "Car Dodge"
#define STR_GAME_SNAKE "Snake"
#define STR_GAME_LIFE "Game Life"

#define STR_WIN "WIN!"
#define STR_GAMEOVER "GAMEOVER!"
#define STR_SCORE "Score:"
#define STR_HIGHSCORE "Highscore:"
#define STR_NEWHIGHSCORE "!NEW HIGHSCORE!"

// Little images (8x8) for showing day of week of next alarm on main screen

#define DOWIMG_MON 0x0F, 0x01, 0x02, 0x01, 0x6F, 0x90, 0x90, 0x60
#define DOWIMG_TUE 0x01, 0x1F, 0x01, 0x00, 0x78, 0x80, 0x80, 0x78
#define DOWIMG_WED 0x0F, 0x04, 0x02, 0x04, 0x0F, 0xF8, 0xA8, 0xA8
#define DOWIMG_THU 0x01, 0x1F, 0x01, 0x00, 0xF8, 0x20, 0x20, 0xF8
#define DOWIMG_FRI 0x1F, 0x05, 0x05, 0x00, 0xF8, 0x28, 0x68, 0x90
#define DOWIMG_SAT 0x12, 0x15, 0x09, 0x00, 0xF0, 0x28, 0x28, 0xF0
#define DOWIMG_SUN 0x12, 0x15, 0x09, 0x00, 0x78, 0x80, 0x80, 0x78

// Character set for this language
// 宽 5 * 8
// 0x5F = 0101 1111 -> 取模是 阴码, 逆向, 低位在前 因为只有一行, 所以不知道是不列行式
// !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
// 找不到好的字体
// 0x5f=0101 1111
#define CHARACTER_SET                                                              \
    {0x00, 0x00, 0x00, 0x00, 0x00},     /* space */                                \
        {0x00, 0x5F, 0x00, 0x00, 0x00}, /* ! */                                    \
        {0x00, 0x07, 0x00, 0x07, 0x00}, /* " */                                    \
        {0x14, 0x7F, 0x14, 0x7F, 0x14}, /* # */                                    \
        {0x24, 0x2A, 0x7F, 0x2A, 0x12}, /* $ */                                    \
        {0x23, 0x13, 0x08, 0x64, 0x62}, /* % */                                    \
        {0x36, 0x49, 0x55, 0x22, 0x50}, /* & */                                    \
        {0x00, 0x03, 0x03, 0x00, 0x00}, /* ' */                                    \
        {0x1C, 0x22, 0x41, 0x00, 0x00}, /* ( */                                    \
        {0x41, 0x22, 0x1C, 0x00, 0x00}, /* ) */                                    \
        {0x08, 0x2A, 0x1C, 0x2A, 0x08}, /* * */                                    \
        {0x08, 0x08, 0x3E, 0x08, 0x08}, /* + */                                    \
        {0xA0, 0x60, 0x00, 0x00, 0x00}, /* , */                                    \
        {0x08, 0x08, 0x08, 0x08, 0x08}, /* - */                                    \
        {0x60, 0x60, 0x00, 0x00, 0x00}, /* . */                                    \
        {0x20, 0x10, 0x08, 0x04, 0x02}, /* / */                                    \
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */                                    \
        {0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */                                    \
        {0x62, 0x51, 0x49, 0x49, 0x46}, /* 2 */                                    \
        {0x22, 0x41, 0x49, 0x49, 0x36}, /* 3 */                                    \
        {0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */                                    \
        {0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */                                    \
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */                                    \
        {0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */                                    \
        {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */                                    \
        {0x06, 0x49, 0x49, 0x29, 0x1E}, /* 9 */                                    \
        {0x00, 0x36, 0x36, 0x00, 0x00}, /* : */                                    \
        {0x00, 0xAC, 0x6C, 0x00, 0x00}, /*  ; */                                   \
        {0x08, 0x14, 0x22, 0x41, 0x00}, /* < */                                    \
        {0x14, 0x14, 0x14, 0x14, 0x14}, /* = */                                    \
        {0x41, 0x22, 0x14, 0x08, 0x00}, /* > */                                    \
        {0x02, 0x01, 0x51, 0x09, 0x06}, /* ? */                                    \
        {0x32, 0x49, 0x79, 0x41, 0x3E}, /* @ */                                    \
        {0x7E, 0x09, 0x09, 0x09, 0x7E}, /* A */                                    \
        {0x7F, 0x49, 0x49, 0x49, 0x36}, /* B */                                    \
        {0x3E, 0x41, 0x41, 0x41, 0x22}, /* C */                                    \
        {0x7F, 0x41, 0x41, 0x22, 0x1C}, /* D */                                    \
        {0x7F, 0x49, 0x49, 0x49, 0x41}, /* E */                                    \
        {0x7F, 0x09, 0x09, 0x09, 0x01}, /* F */                                    \
        {0x3E, 0x41, 0x41, 0x51, 0x72}, /* G */                                    \
        {0x7F, 0x08, 0x08, 0x08, 0x7F}, /* H */                                    \
        {0x41, 0x7F, 0x41, 0x00, 0x00}, /* I */                                    \
        {0x20, 0x40, 0x41, 0x3F, 0x01}, /* J */                                    \
        {0x7F, 0x08, 0x14, 0x22, 0x41}, /* K */                                    \
        {0x7F, 0x40, 0x40, 0x40, 0x40}, /* L */                                    \
        {0x7F, 0x02, 0x0C, 0x02, 0x7F}, /* M */                                    \
        {0x7F, 0x04, 0x08, 0x10, 0x7F}, /* N */                                    \
        {0x3E, 0x41, 0x41, 0x41, 0x3E}, /* O */                                    \
        {0x7F, 0x09, 0x09, 0x09, 0x06}, /* P */                                    \
        {0x3E, 0x41, 0x51, 0x21, 0x5E}, /* Q */                                    \
        {0x7F, 0x09, 0x19, 0x29, 0x46}, /* R */                                    \
        {0x26, 0x49, 0x49, 0x49, 0x32}, /* S */                                    \
        {0x01, 0x01, 0x7F, 0x01, 0x01}, /* T */                                    \
        {0x3F, 0x40, 0x40, 0x40, 0x3F}, /* U */                                    \
        {0x1F, 0x20, 0x40, 0x20, 0x1F}, /* V */                                    \
        {0x3F, 0x40, 0x38, 0x40, 0x3F}, /* W */                                    \
        {0x63, 0x14, 0x08, 0x14, 0x63}, /* X */                                    \
        {0x03, 0x04, 0x78, 0x04, 0x03}, /* Y */                                    \
        {0x61, 0x51, 0x49, 0x45, 0x43}, /* Z */                                    \
        {0x7F, 0x41, 0x41, 0x00, 0x00}, /* [ */                                    \
        {0x02, 0x04, 0x08, 0x10, 0x20}, /* \ */                                    \
        {0x41, 0x41, 0x7F, 0x00, 0x00}, /* ] */                                    \
        {0x04, 0x02, 0x01, 0x02, 0x04}, /* ^ */                                    \
        {0x80, 0x80, 0x80, 0x80, 0x80}, /* _ */                                    \
        {0x01, 0x02, 0x04, 0x00, 0x00}, /* ' */                                    \
        {0x20, 0x54, 0x54, 0x54, 0x78}, /* a */                                    \
        {0x7F, 0x48, 0x44, 0x44, 0x38}, /* b */                                    \
        {0x38, 0x44, 0x44, 0x28, 0x00}, /* c */                                    \
        {0x38, 0x44, 0x44, 0x48, 0x7F}, /* d */                                    \
        {0x38, 0x54, 0x54, 0x54, 0x18}, /* e */                                    \
        {0x08, 0x7E, 0x09, 0x02, 0x00}, /* f */                                    \
        {0x18, 0xA4, 0xA4, 0xA4, 0x7C}, /* g */                                    \
        {0x7F, 0x08, 0x04, 0x04, 0x78}, /* h */                                    \
        {0x00, 0x7D, 0x00, 0x00, 0x00}, /* i */                                    \
        {0x80, 0x84, 0x7D, 0x00, 0x00}, /* j */                                    \
        {0x7F, 0x10, 0x28, 0x44, 0x00}, /* k */                                    \
        {0x41, 0x7F, 0x40, 0x00, 0x00}, /* l */                                    \
        {0x7C, 0x04, 0x18, 0x04, 0x78}, /* m */                                    \
        {0x7C, 0x08, 0x04, 0x7C, 0x00}, /* n */                                    \
        {0x38, 0x44, 0x44, 0x38, 0x00}, /* o */                                    \
        {0xFC, 0x24, 0x24, 0x18, 0x00}, /* p */                                    \
        {0x18, 0x24, 0x24, 0xFC, 0x00}, /* q */                                    \
        {0x00, 0x7C, 0x08, 0x04, 0x00}, /* r */                                    \
        {0x48, 0x54, 0x54, 0x24, 0x00}, /* s */                                    \
        {0x04, 0x7F, 0x44, 0x00, 0x00}, /* t */                                    \
        {0x3C, 0x40, 0x40, 0x7C, 0x00}, /* u */                                    \
        {0x1C, 0x20, 0x40, 0x20, 0x1C}, /* v */                                    \
        {0x3C, 0x40, 0x30, 0x40, 0x3C}, /* w */                                    \
        {0x44, 0x28, 0x10, 0x28, 0x44}, /* x */                                    \
        {0x1C, 0xA0, 0xA0, 0x7C, 0x00}, /* y */                                    \
        {0x44, 0x64, 0x54, 0x4C, 0x44}, /* z */                                    \
        {0x08, 0x36, 0x41, 0x00, 0x00}, /* { */                                    \
        {0x00, 0x7F, 0x00, 0x00, 0x00}, /* | */                                    \
        {0x41, 0x36, 0x08, 0x00, 0x00}, /* } */                                    \
        {0x02, 0x01, 0x01, 0x02, 0x01}, /* ~ */                                    \
                                                                                   \
        {0x02, 0x05, 0x05, 0x02, 0x00}, /* degrees (non-standard, normally DEL) */ \
        {0x60, 0xC0, 0xF0, 0x38, 0x1C}, /* tick (non-standard) */

#endif /* ENGLISH_H_ */
