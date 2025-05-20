/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2014 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

#if TUNEMEM_TYPE == TUNEMEM_PROGMEM
#define tune_read(x) (pgm_read_word(&x))
#elif TUNEMEM_TYPE == TUNEMEM_EEPROM
#define tune_read(x) (eeprom_read_word((const uint16_t *)&x))
#else
#define tune_read(x) (x)
#endif

static byte idx;		   // Position in tune
static const tune_t *tune; // The tune
static vol_t vol;		   // Volume
static tonePrio_t prio;	   // Priority

static void next(void);

void tune_play(const tune_t *_tune, vol_t _vol, tonePrio_t _prio)
{
	// Check priority, if lower than currently playing tune priority then ignore it
	if (_prio < prio)
		return;

	prio = _prio;
	tune = _tune;
	vol = _vol;
	idx = 0;

	// Begin playing
	next();
}

void tune_stop(tonePrio_t _prio)
{
	buzzer_buzz(0, TONE_STOP, VOL_OTHER, _prio, NULL);
	prio = PRIO_MIN;
}

static void next()
{
	// Read next tone
	tune_t data = tune_read(tune[idx++]);

	uint16_t len = data; // 取低16位
	if (len != TONE_REPEAT)
	{
		// Play next tone
		// 高16位是曲调，低16位是节拍时长
		buzzer_buzz(len, (data >> 16), vol, prio, next);

		// If tone was TONE_STOP then reset priority
		if (len == TONE_STOP)
		{
			prio = PRIO_MIN;
		}
	}
	else
	{
		// Repeat
		idx = 0;
		next();
	}
}

enum
{
    LA = 262,
    LB = 294,
    LC = 330,
    LD = 349,
    LE = 392,
    LF = 440,
    LG = 494,

    MA = 523,
    MB = 578,
    MC = 659,
    MD = 698,
    ME = 784,
    MF = 880,
    MG = 988,

    HA = 1064,
    HB = 1175,
    HC = 1318,
    HD = 1397,
    HE = 1568,
    HF = 1760,
    HG = 1976
};

const uint32_t STAY[] = {

    // 5353
    ME << 16 | 250,
    MC << 16 | 250,
    ME << 16 | 250,
    MC << 16 | 250,
    // 222321
    MB << 16 | 125,
    MB << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,
    MB << 16 | 125,
    MA << 16 | 250,
    // 7115
    LG << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 125,
    ME << 16 | 500,
    // 177777111
    MA << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 250,
    // 1715
    MA << 16 | 125,
    LG << 16 | 125,
    MA << 16 | 125,
    ME << 16 | 500,
    // 177777111
    MA << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 250,
    // 1715
    MA << 16 | 125,
    LG << 16 | 125,
    MA << 16 | 125,
    ME << 16 | 500,
    // 177777111
    MA << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    LG << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 250,
    // 1715
    MA << 16 | 125,
    LG << 16 | 125,
    MA << 16 | 125,
    ME << 16 | 500,
    // 71275
    LG << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MG << 16 | 125,
    ME << 16 | 500,

    TONE_REPEAT

}; // 旋律

const uint32_t TUNE[] = {

    LF << 16 | 250,
    LC << 16 | 250,
    HF << 16 | 250,
    MC << 16 | 250,

    LD << 16 | 250,
    MA << 16 | 250,
    MD << 16 | 250,
    MA << 16 | 250,

    LE << 16 | 250,
    MB << 16 | 250,
    ME << 16 | 250,
    MB << 16 | 250,

    MA << 16 | 250,
    ME << 16 | 250,
    HA << 16 | 250,
    ME << 16 | 250,

    LF << 16 | 250,
    LC << 16 | 250,
    HF << 16 | 250,
    MC << 16 | 250,

    LD << 16 | 250,
    MA << 16 | 250,
    MD << 16 | 250,
    MA << 16 | 250,

    LE << 16 | 250,
    MB << 16 | 250,
    ME << 16 | 250,
    MB << 16 | 250,
    // 1 5123
    MA << 16 | 500,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,
    // 2111
    MB << 16 | 250,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 500,

    TONE_PAUSE << 16 | 125,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,

    // 212233
    MB << 16 | 250,
    MA << 16 | 125,
    MB << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 250,
    MC << 16 | 125,

    // 35123
    MC << 16 | 500,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,

    // 2111
    MB << 16 | 250,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 500,

    // 05123
    TONE_PAUSE << 16 | 125,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,
    // 212253
    MB << 16 | 250,
    MA << 16 | 125,
    MB << 16 | 125,
    MB << 16 | 125,
    ME << 16 | 250,
    MC << 16 | 125,

    // 334
    MC << 16 | 500,
    MC << 16 | 250,
    MD << 16 | 250,

    // 55555
    ME << 16 | 250,
    ME << 16 | 125,
    ME << 16 | 125,
    ME << 16 | 250,
    ME << 16 | 250,

    // 531134
    ME << 16 | 250,
    MC << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 125,
    MC << 16 | 250,
    MD << 16 | 125,

    // 55555
    ME << 16 | 250,
    ME << 16 | 125,
    ME << 16 | 125,
    ME << 16 | 250,
    ME << 16 | 250,
    // 531 112
    ME << 16 | 250,
    MC << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 250,
    MA << 16 | 125,
    MB << 16 | 125,

    // 33333
    MC << 16 | 250,
    MC << 16 | 125,
    MC << 16 | 125,
    MC << 16 | 250,
    MC << 16 | 250,
    // 366321
    MC << 16 | 250,
    LF << 16 | 250,
    MF << 16 | 125,
    MC << 16 | 125,
    MB << 16 | 125,
    LA << 16 | 250,

    // 20
    MB << 16 | 500,
    TONE_PAUSE << 16 | 125,

    // 05123
    TONE_PAUSE << 16 | 125,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,
    // 2111
    MB << 16 | 250,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 500,
    // 05123
    TONE_PAUSE << 16 | 125,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,

    // 212233
    MB << 16 | 250,
    MA << 16 | 125,
    MB << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 250,
    MC << 16 | 125,

    // 35123
    MC << 16 | 500,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,
    // 2111
    MB << 16 | 250,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 500,
    // 05123
    TONE_PAUSE << 16 | 125,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,
    // 212253
    MB << 16 | 250,
    MA << 16 | 125,
    MB << 16 | 125,
    MB << 16 | 125,
    ME << 16 | 250,
    MC << 16 | 125,

    // 334
    MC << 16 | 500,
    MC << 16 | 250,
    MD << 16 | 250,

    // 55555
    ME << 16 | 250,
    ME << 16 | 125,
    ME << 16 | 125,
    ME << 16 | 250,
    ME << 16 | 250,

    // 531134
    ME << 16 | 250,
    MC << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 125,
    MC << 16 | 250,
    MD << 16 | 125,

    // 55555
    ME << 16 | 250,
    ME << 16 | 125,
    ME << 16 | 125,
    ME << 16 | 250,
    ME << 16 | 250,
    // 531 112
    ME << 16 | 250,
    MC << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 250,
    MA << 16 | 125,
    MB << 16 | 125,

    // 33333
    MC << 16 | 250,
    MC << 16 | 125,
    MC << 16 | 125,
    MC << 16 | 250,
    MC << 16 | 250,
    // 363216
    MC << 16 | 500,
    MF << 16 | 125,
    MC << 16 | 125,
    MB << 16 | 125,
    MA << 16 | 125,
    LF << 16 | 125,

    // 10
    MA << 16 | 500,
    TONE_PAUSE << 16 | 250,
    // 0
    // TONE_PAUSE<<16 | 3000,

    // 55555
    ME << 16 | 250,
    ME << 16 | 125,
    ME << 16 | 125,
    ME << 16 | 250,
    ME << 16 | 250,

    // 531134
    ME << 16 | 250,
    MC << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 125,
    MC << 16 | 250,
    MD << 16 | 125,

    // 55555
    ME << 16 | 250,
    ME << 16 | 125,
    ME << 16 | 125,
    ME << 16 | 250,
    ME << 16 | 250,
    // 531 112
    ME << 16 | 250,
    MC << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 250,
    MA << 16 | 125,
    MB << 16 | 125,

    // 33333
    MC << 16 | 250,
    MC << 16 | 125,
    MC << 16 | 125,
    MC << 16 | 250,
    MC << 16 | 250,
    // 366321
    MC << 16 | 250,
    LF << 16 | 250,
    MF << 16 | 125,
    MC << 16 | 125,
    MB << 16 | 125,
    LA << 16 | 250,

    // 20
    MB << 16 | 500,
    TONE_PAUSE << 16 | 125,

    // 05123
    TONE_PAUSE << 16 | 125,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,
    // 2111
    MB << 16 | 250,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 500,
    // 05123
    TONE_PAUSE << 16 | 125,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,

    // 212233
    MB << 16 | 250,
    MA << 16 | 125,
    MB << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 250,
    MC << 16 | 125,

    // 35123
    MC << 16 | 500,
    LE << 16 | 125,
    MA << 16 | 125,
    MB << 16 | 125,
    MC << 16 | 125,

    // 2111
    MB << 16 | 250,
    MA << 16 | 125,
    MA << 16 | 125,
    MA << 16 | 500,

    TONE_PAUSE << 16 | 2000,

    TONE_REPEAT

}; // 旋律
