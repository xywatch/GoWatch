/*
 * Project: N|Watch
 * Author: Zak Kemble, contact@zakkemble.co.uk
 * Copyright: (C) 2013 by Zak Kemble
 * License: GNU GPL v3 (see License.txt)
 * Web: http://blog.zakkemble.co.uk/diy-digital-wristwatch/
 */

#include "common.h"

#if COMPILE_ANIMATIONS

static anim_s animationStatus;

// http://javascript.info/tutorial/animation

void animation_init()
{
    animationStatus.active = false;
    animationStatus.animOnComplete = NULL;
}

void animation_update()
{
    if (animationStatus.active)
    {
        byte offsetY = animationStatus.offsetY;
        // �������������˳�����
        if (animationStatus.goingOffScreen)
        {
            if (offsetY < 4)
            {
                offsetY += 1;
            }
            else if (offsetY < 8)
            {
                offsetY += 3;
            }
            else if (offsetY < 16)
            {
                offsetY += 5;
            }
            else
            {
                offsetY += 8;
            }

            if (offsetY >= FRAME_HEIGHT)
            {
                animationStatus.active = false;
                offsetY = 0;
            }
        }
        // �����������½������
        else
        {
            if (offsetY > 255 - 4)
            {
                offsetY += 1;
            }
            else if (offsetY > 255 - 8)
            {
                offsetY += 3;
            }
            else if (offsetY > 255 - 16)
            {
                offsetY += 5;
            }
            else
            {
                offsetY += 8;
            }

            if (offsetY < 10)
            {
                animationStatus.active = false;
                offsetY = 0;
            }
        }

        animationStatus.offsetY = offsetY;

        // ��������, ִ�ж�����������
        if (!animationStatus.active && animationStatus.animOnComplete != NULL)
        {
            animationStatus.animOnComplete();
            animationStatus.animOnComplete = NULL;
        }
    }
}

// ��ʼ��������
// ������animOnComplete ������ָ��ָ������������������ִ�еĺ���, 
// animOnComplete() ִ��ʱҲ��
// goingOffScreen=
// #define ANIM_MOVE_OFF true // ANIM_MOVE_ON�������������˳�
// #define ANIM_MOVE_ON false // ANIM_MOVE_ON�����������½���
/*
����1:
���̽���, ����, ���� altitude_open
bool altitude_open(void)
{
    display_setDrawFunc(draw);
    buttons_setFuncs(btnup, btnExit, btndown);
    animation_start(null, ANIM_MOVE_ON);
    return true;
}
��ʱ������������ֱ�ӽ���altitude, ����ı���ֱ����ʧ��, ��û�б����˳��Ķ���

�������м�����ʱ:
static bool btnExit()
{
    animation_start(display_load, ANIM_MOVE_OFF);
}
altitude�ȴ��������˳�����, Ȼ��ִ��display_load, ���̴������½������, �൱���ٲ���Ч��


����2:
���̽���, ���м�, ���� menu����
1. ����ִ���˳�OFF����, Ȼ��ִ��mOpen
2. mOpenִ�ж���ON, 
������һ�������Ǳ����ȴ��������˳�, menu�������½���, �൱���ٲ���Ч��

void mMainOpen()
{
    buttons_setFuncs(NULL, menu_select, NULL);
    animation_start(mOpen, ANIM_MOVE_OFF);
}

// �����˵�
static void mOpen()
{
    display_setDrawFunc(menu_draw); // �󶨻��ƺ���Ϊmenu_draw

    buttons_setFuncs(menu_up, menu_select, menu_down); // �󶨰������ܺ���

    setMenuInfo(OPTION_COUNT, MENU_TYPE_ICON, PSTR(STR_MAINMENU));   // ��ȡ��ǰ�˵���Ϣ��ѡ��������˵����������ֻ���ͼ�꣩
    setMenuFuncs(MENUFUNC_NEXT, mSelect, MENUFUNC_PREV, itemLoader); // �󶨲˵��ĺ���,��ǰ������ѡ��ȷ��

    setPrevMenuOpen(&prevMenuData, mOpen); // �����ϼ��˵�

    animation_start(null, ANIM_MOVE_ON);
}

*/
void animation_start(void (*animOnComplete)(void), bool goingOffScreen)
{
    if (appConfig.animations)
    {
        animationStatus.active = true;
        animationStatus.offsetY = goingOffScreen ? 0 : 192;
        animationStatus.animOnComplete = animOnComplete;
        animationStatus.goingOffScreen = goingOffScreen;
    }

    else
    {
        if (animOnComplete != NULL)
        {
            animOnComplete();
        }
    }
}

bool animation_active()
{
    return animationStatus.active;
}

bool animation_movingOn()
{
    return !animationStatus.goingOffScreen;
}

byte animation_offsetY()
{
    return animationStatus.offsetY;
}

#else

void animation_start(void (*animOnComplete)(void), bool goingOffScreen)
{
    (void)(goingOffScreen);

    if (animOnComplete != NULL)
    {
        animOnComplete();
    }
}

#endif
