#include "common.h"
#include "gamelife.h"

static bool btnExit(void);
static bool btnRight(void);
static bool btnLeft(void);
display_t game_life_draw(void);

/*  Defines  */

enum MODE
{
	TITLE_MODE,
	GAME_MODE
};

static enum MODE mode = TITLE_MODE;
bool isDone;
byte btnType;

// �м�
static bool btnExit()
{
		btnType = BTN_FUNC;

    // animation_start(display_load, ANIM_MOVE_OFF);
    return true;
}

// ��
static bool btnLeft()
{
		btnType = BTN_LEFT;
    return true;
}

// ��
static bool btnRight()
{
		btnType = BTN_RIGHT;
    return true;
}

// ��ʼֻ����һ��
void game_life_start()
{
    menu_close();

    display_setDrawFunc(game_life_draw);
    buttons_setFuncs(btnRight, btnExit, btnLeft);
		
		mode = TITLE_MODE;
		initTitle();
}

// ��ÿһ֡������
display_t game_life_draw()
{
		// title mode
		if (mode == TITLE_MODE) {
			bool isDone = updateTitle(btnType);
			drawTitle();
			// ���gameģʽ
			if (isDone)
			{
				mode = GAME_MODE;
				initGame();
				// �����ͷ��title, �ٱ���
				clearHeader();
				// ��������һ�ε�״̬
				computeAllPointsStatus();
			}
		}
		// game mode
		else {
			bool isDone = updateGame(btnType); // ûʲô����
			drawGame();
			// ���title mode
			if (isDone) {
				mode = TITLE_MODE;
				initTitle();
			}
			else if (btnType == BTN_RIGHT) {
				// �ر�
				exitMeThenRun(display_load);
			}
		}

		// ��ԭbtn
		btnType = 0;

		return DISPLAY_BUSY;  //������Ļˢ��æ
}
