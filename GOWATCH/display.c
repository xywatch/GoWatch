/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

// Frame rate when stuff is happening
// If this is too low then animations will be jerky
// Animations are also frame rate based instead of time based, adjusting frame rate will also effect animation speed
#define FRAME_RATE 60

// Frame rate when nothing is going on
// If this is too low then the interface will seem a bit slow to respond
#define FRAME_RATE_LOW 25

#define FRAME_RATE_MS ((byte)(1000 / FRAME_RATE))
#define FRAME_RATE_LOW_MS ((byte)(1000 / FRAME_RATE_LOW))

static draw_f drawFunc; // ������Ļ�ĺ���
static display_f func; // ��watch face ����

byte MY_FPS = FRAME_RATE; // ȫ�ֶ���֡�ʿ���

typedef struct
{
    byte height; // �����߶�
    bool closing; // ���� �� �ػ�����
    bool doingLine; // �Ƿ�����ִ�ж���
    byte lineWidth; // �������
    byte lineClosing; // �Ƿ����ڹرն���
    bool active; // �����Ƿ�����ִ��
} crt_anim_s;

static crt_anim_s crt_anim;

static void crt_animation(void);

bool display_is_ani_active()
{
    return crt_anim.active || animation_active();
}

void display_set(display_f faceFunc)
{
    func = faceFunc;
}

// ��watch face ����
void display_load()
{
    if (func != NULL)
    {
        func();
    }
}

// ���û�����Ļ�ĺ���
draw_f display_setDrawFunc(draw_f func)
{
    draw_f old = drawFunc; // ����ɵĻ�����Ļ�ĺ���
    drawFunc = func; // �����µĻ�����Ļ�ĺ���
    return old; // ���ؾɵĻ�����Ļ�ĺ���
}

// ˢ����Ļ
// main.c���һֱ����, ���Ի�һֱˢ����Ļ
/*
����ϵͳ״̬��̬����֡��
����ʱʹ�õ�֡�ʣ�25fps����ʡ��Դ
�ж���ʱʹ�ø�֡�ʣ�60fps����֤����
�����������ʾϵͳ�ĺ��ĸ��º���������
1. ������ʾˢ��Ƶ��
2. ������״̬
3. ������Ʋ���
4. ʵ��CRT����Ч��
5. �ṩFPS��ʾ����
6. ��̬����֡����ƽ�����ܺ͹���
�����������ѭ���б�Ƶ�����ã�ȷ����ʾ�����ܹ���ʱ���£�ͬʱͨ��֡�ʿ��Ʊ����������ϵͳ��Դ��
*/
void display_update()
{
    static millis8_t lastDraw; // ��һ��ˢ����Ļ��ʱ��
    static byte fpsMs, fpsms;  // ˢ����Ļ�ļ��ʱ��

    // Limit frame rate
    millis8_t now = millis();

    // �����ǰʱ������һ��ˢ����Ļ��ʱ���С��ˢ����Ļ�ļ��ʱ��, ��ˢ����Ļ
    if ((millis8_t)(now - lastDraw) < fpsMs)
    {
        // pwrmgr_setState(PWR_ACTIVE_DISPLAY, PWR_STATE_IDLE);
        return;
    }

    fpsms = now - lastDraw;
    lastDraw = now;

    // debugPin_draw(HIGH);

    display_t busy = DISPLAY_DONE;

    // ���¶���
    animation_update();

    // ������Ļ
    if (drawFunc != NULL && (crt_anim.active || (!crt_anim.active && !crt_anim.closing)))
    {
        busy = drawFunc();
    }

    // ִ�п����͹�������
    if (crt_anim.active)
    {
        crt_animation();
    }

    // ��������͹�����������ִ��, ��������Ļˢ��״̬Ϊˢ����
    if (crt_anim.active || animation_active())
    {
        busy = DISPLAY_BUSY;
    }

    // if (appConfig.showFPS)
    // {
    //     // Work out & draw FPS, add 2ms (actually 2.31ms) for time it takes to send to OLED, clear buffer etc
    //     // This is only approximate
    //     // millis8_t end = millis() + 1;
    //     char buff[5];
    //     sprintf_P(buff, PSTR("%u"), (uint)(1000 / fpsms));
    //     //	draw_string(buff,false,107,56);
    //     draw_string(buff, false, 100, 56);
    // }

    // End drawing, send to OLED
    draw_end();

    //	debugPin_draw(LOW);

    // ����֡��
    // �����Ļˢ�����, ������֡��Ϊ��֡��
    if (busy == DISPLAY_DONE)
    {
        //	pwrmgr_setState(PWR_ACTIVE_DISPLAY, PWR_STATE_NONE);
        fpsMs = FRAME_RATE_LOW_MS;
    }
    // �����Ļˢ����, ������֡��Ϊ��֡��
    else
    {
        //	pwrmgr_setState(PWR_ACTIVE_DISPLAY, PWR_STATE_IDLE);
        fpsMs = (byte)(1000 / MY_FPS);
        // #if COMPILE_GAME1
        //     if (drawFunc == game1_draw)
        //       fpsMs <<= 1;
        // #endif
    }
}

void display_startCRTAnim(crtAnim_t open)
{
    if (!appConfig.animations)
    {
        crt_anim.active = false;
        return;
    }

    // ��������
    // �м����Ϻ�����չ��
    if (open == CRTANIM_OPEN)
    {
        crt_anim.closing = false;    // ��������
        crt_anim.doingLine = true;   // ֱ�ӿ�ʼɨ����Ч��
        crt_anim.height = FRAME_HEIGHT / 2;  // �߶ȴ���Ļ�м俪ʼ
        crt_anim.lineClosing = false;  // ɨ������չ��״̬
        crt_anim.lineWidth = 0;       // ɨ���߿�ȴ�0��ʼ
    }
    // ��������
    // �������м���£
    else
    {
        crt_anim.closing = true;
        crt_anim.doingLine = false;
        crt_anim.height = 0;
        crt_anim.lineClosing = true; // ɨ����������״̬
        crt_anim.lineWidth = FRAME_WIDTH;
    }

    crt_anim.active = true;
}

// ִ�п����͹�������
// �������Ч��ģ������ʽ CRT ��ʾ���Ŀ����͹���Ч����
// 1. ����ʱ�����м�������չ����Ȼ����һ��ɨ����Ч��
// 2. ����ʱ���ȳ���ɨ���ߣ�Ȼ����������м�����
// ����ͨ��������ʾ��������oledBuffer����ʵ�֣�ͨ����ȷ����ÿ�����ص�״̬������ƽ���Ĺ���Ч����
// ���ֶ����������ۣ������ܹ����û�һ����ȷ���Ӿ������������豸���ڽ�����˳�˯��״̬��
/*
����������������������Ȼ��߶ȶ���
1. ����Ļ�м俪ʼ��height = FRAME_HEIGHT / 2��
2. ֱ�ӿ�ʼɨ����Ч����doingLine = true��
3. ɨ���ߴ��м�������չ����lineWidth ��0��ʼ���ӣ�
4. ��ɨ���ߴﵽ��Ļ���ʱlineWidth=FRAME_WIDTH�������л��ɸ߶ȶ���, doingLine = false
5. �߶ȶ�������Ļ�м俪ʼ��height = FRAME_HEIGHT / 2��

������ʵ�ָ����� CRT ��ʾ���Ŀ���Ч��������Ļ�м����һ��ɨ���ߣ�Ȼ��������չ����

�����������ȸ߶ȶ�����Ȼ����������
1. ����Ļ�������˿�ʼ������height ��0��ʼ���ӣ�
2. ����������Ļ�м�ʱ��height = FRAME_HEIGHT / 2�����л���ɨ����Ч��
3. ɨ���ߴ��������м�������lineWidth ����Ļ��ȿ�ʼ���٣�
4. ��ɨ������ȫ����ʱ��lineWidth = 0������������
������ʵ��ģ���� CRT ��ʾ���Ĺ���Ч�������Ǵ��������м�������Ȼ�����һ��ɨ���ߣ����ɨ���ߴ��������м�������ʧ��
*/
static void crt_animation()
{
    byte height = crt_anim.height;
    byte lineWidth = crt_anim.lineWidth;

    // �߶ȶ���
    if (!crt_anim.doingLine)
    {
        // �����ǿ������ǹ��������߶ȱ仯����
        if (crt_anim.closing)
        {
            // ����ʱ, �߶�����
            height += 3;
        }
        else
        {
            // ����ʱ, �߶ȼ���
            height -= 3;
        }

        // ���߶ȴﵽ��Ļһ��ʱ���л���ɨ����Ч��
        // ֻ�й���ʱ���߶ȶ����Ż�ﵽ��Ļһ��, ֻ�й���ʱ�Ż�ִ���±ߵĴ���
        if (height >= FRAME_HEIGHT / 2)
        {
            // �������, ��߶�����Ϊһ��, ���л�����������
            if (crt_anim.closing)
            {
                height = FRAME_HEIGHT / 2;
                crt_anim.doingLine = true; // �л�����������, ɨ���ߴ��������м�����
            }
            // �������, ��߶�����Ϊ0, ���رն���
            // ��ִ������Ĵ���? ����
            else
            {
                height = 0;
                crt_anim.active = false;
            }
            // crt_anim.closing = !crt_anim.closing;
        }
    }
    // �������� crt_anim.doingLine = true
    else
    {
        // �����ǿ������ǹ�������������ȱ仯
        if (crt_anim.lineClosing)
        {
            lineWidth -= 6; // ����ʱ��������
        }
        else
        {
            lineWidth += 10; // ����ʱ��������
        }

        // ֻ�п���ʱ��������ȲŻ�ﵽ���, �Ż�ִ���±ߵĴ���
        // ���������ȴﵽ���
        if (lineWidth >= FRAME_WIDTH)
        {
            // �������״̬�߿�ȱ�Ϊ0
            // ������벻��ִ��
            if (crt_anim.lineClosing)
            {
                lineWidth = 0;
            }
            // ����״̬�߿�ȱ�Ϊ��Ļ���
            else
            {
                lineWidth = FRAME_WIDTH;
            }

            // ���ɨ������չ��״̬�����л����߶ȶ���
            if (!crt_anim.lineClosing)
            {
                crt_anim.doingLine = false;
            }

            // ���ɨ����������״̬�������ǹ�����������رն���
            // ������벻��ִ��, lineClosing = true��ʾ�ǹ�������
            if (crt_anim.lineClosing && crt_anim.closing)
            {
                crt_anim.active = false;
            }

            // ״̬ת��? ��ʲô��
            crt_anim.lineClosing = !crt_anim.lineClosing;
        }
    }

    // Full rows 
    // ���ݸ߶ȼ�����Ҫ��������
    byte rows = height / 8;
    LOOP(rows, i)
    {
        // ����������˵���
        memset(&oledBuffer[i * FRAME_WIDTH], 0, FRAME_WIDTH);
        memset(&oledBuffer[FRAME_BUFFER_SIZE - FRAME_WIDTH - (i * FRAME_WIDTH)], 0, FRAME_WIDTH);
    }

    // ���Ʋ����кͱ�Ե��
    byte prows = height % 8;

    if (prows)
    { // Partial rows & edge line   
        // ������ʼ�ͽ�������
        uint idxStart = rows * FRAME_WIDTH;
        uint idxEnd = ((FRAME_BUFFER_SIZE - 1) - idxStart);
        // ���㽥���Ե
        byte a = (255 << prows);
        byte b = (255 >> prows);
        byte c = (1 << prows);
        byte d = (128 >> prows);
        // ���ƽ����Ե
        LOOP(FRAME_WIDTH, i)
        {
            oledBuffer[idxStart] = (oledBuffer[idxStart] & a) | c;
            idxStart++;

            oledBuffer[idxEnd] = (oledBuffer[idxEnd] & b) | d;
            idxEnd--;
        }
    }
    else if (height)
    { // Edge line
        uint pos = ((byte)(FRAME_WIDTH - lineWidth) / 2) + ((byte)(FRAME_HEIGHT - height) / 8) * FRAME_WIDTH;
        memset(&oledBuffer[pos], 0x01, lineWidth);

        if (height != FRAME_HEIGHT / 2)
        {
            pos = (height / 8) * FRAME_WIDTH;
            LOOPR(FRAME_WIDTH, x)
            oledBuffer[pos + x] |= 0x01;
        }
    }

    crt_anim.height = height;
    crt_anim.lineWidth = lineWidth;

    //	if(crt_anim.doingLine && crt_anim.closing)
    //		draw_bitmap_s2(&crtdotImage);
}
