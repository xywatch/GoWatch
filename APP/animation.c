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
        // 动画从上往下退出界面
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
        // 动画从上往下进入界面
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

        // 动画结束, 执行动画结束函数
        if (!animationStatus.active && animationStatus.animOnComplete != NULL)
        {
            animationStatus.animOnComplete();
            animationStatus.animOnComplete = NULL;
        }
    }
}

// 开始动画函数
// 参数：animOnComplete ，函数指针指向函数，动画过结束后执行的函数, 
// animOnComplete() 执行时也会
// goingOffScreen=
// #define ANIM_MOVE_OFF true // ANIM_MOVE_ON动画从上往下退出
// #define ANIM_MOVE_ON false // ANIM_MOVE_ON动画从上往下进入
/*
案例1:
表盘界面, 按上, 进入 altitude_open
bool altitude_open(void)
{
    display_setDrawFunc(draw);
    buttons_setFuncs(btnup, btnExit, btndown);
    animation_start(null, ANIM_MOVE_ON);
    return true;
}
此时动画从上往下直接进入altitude, 这里的表盘直接消失了, 并没有表盘退出的动画

当按下中键返回时:
static bool btnExit()
{
    animation_start(display_load, ANIM_MOVE_OFF);
}
altitude先从上往下退出界面, 然后执行display_load, 表盘从上往下进入界面, 相当于瀑布流效果


案例2:
表盘界面, 按中键, 进入 menu界面
1. 表盘执行退出OFF动画, 然后执行mOpen
2. mOpen执行动画ON, 
所以有一个动画是表盘先从上往下退出, menu从上往下进入, 相当于瀑布流效果

void mMainOpen()
{
    buttons_setFuncs(NULL, menu_select, NULL);
    animation_start(mOpen, ANIM_MOVE_OFF);
}

// 打开主菜单
static void mOpen()
{
    display_setDrawFunc(menu_draw); // 绑定绘制函数为menu_draw

    buttons_setFuncs(menu_up, menu_select, menu_down); // 绑定按键功能函数

    setMenuInfo(OPTION_COUNT, MENU_TYPE_ICON, PSTR(STR_MAINMENU));   // 获取当前菜单信息（选项个数，菜单类型是文字还是图标）
    setMenuFuncs(MENUFUNC_NEXT, mSelect, MENUFUNC_PREV, itemLoader); // 绑定菜单的函数,如前进后退选择确认

    setPrevMenuOpen(&prevMenuData, mOpen); // 储存上级菜单

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
