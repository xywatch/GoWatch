#include "common.h"

#if COMPILE_GAME_SNAKE

#define UP 3
#define DOWN 4
#define LEFT 1
#define RIGHT 2

const byte startPosX = 0;
const byte startPosY = 0;
const byte snakeWidth = 32; // * 4 = 128
const byte snakeHeight = 13; // * 4 = 52

int map[snakeWidth][snakeHeight] = {0}; // 地图大小  x-y 
int snake_Grid[snakeWidth][2] = {0}; // 蛇坐�?
byte initSnakeLength = 3;
int snakeLength;
int snakeScore;
int snakeDirection; // 蛇的方向
bool snakeEated = false;

byte renderI = 0;

void Creat_Map(void); // 初始化地图边�?
void Move(void); // 移动
void Paint_Map(int x, int y, int z); // 绘制地图
void Paint_Map2(void);
void Paint_Food(int x,int y ); // 绘制食物
void Paint_Body(int x,int y ); // 绘制身体
void drawSnake(void); // 画蛇
void Snake_Init(void); //蛇及食物初始�?
void Paint_Clean(int x,int y ); //清除�?
void Get_Command(void ); //获取键盘�?
void Grid_Bound(void); //坐标限定
bool GameOver(void); //游戏结束
void Food(void); //生成食物
int  Chek(int i,int j); //检查地图空�?
void Eat_Food(void); //吃食�?
void Show_Score(void); //显示分数

static bool btnExit(void);
static bool btnRight(void);
static bool btnLeft(void);
display_t game_snake_draw(void);

/*
---------------------x 32
|
|
|
y 12

-2 map 四周�?
-1 food
2 body
0 空区

每个点对应到屏幕�?4x4像素
围墙�? 4px ..**
body *..*
food .**.
*/

void Snake_Refresh() //界面刷新
{
	int i, j, temp;
	for (i = 0; i < snakeWidth; i++)
	{
		for (j = 0; j < snakeHeight; j++)
		{
			temp = map[i][j];
			if (temp == 2)
			{
				Paint_Body(i, j);
			}
			else if (temp == -2 || temp == -3 || temp == -4 || temp == -5)
			{
				// Paint_Map(i, j, temp);
			}
			else if (temp == -1)
			{
				Paint_Food(i, j);
			}
			else if (temp == 0)
			{
				Paint_Clean(i, j);
			}
		}
	}
    Paint_Map2();
}

// 只画2像素的墙
void Paint_Map2()
{
    int i, j;

    byte leftX = 3;
    byte topY = 3;
    byte rightX = snakeWidth * 4 - 4;
    byte bottomY = snakeHeight * 4 - 4;

    // �?
    draw_fill_screen(leftX, topY, rightX, topY, 1);
    // �?
    draw_fill_screen(leftX, bottomY, rightX, bottomY, 1);

    // �?
    draw_fill_screen(leftX, topY, leftX, bottomY, 1);
    // �?
    draw_fill_screen(rightX, topY, rightX, bottomY, 1);
}

/*
绘制地图 4px
  -2
-4  -5
  -3
*/
void Paint_Map(int x, int y, int mode)
{
	int i, j;

    for (j = 4 * x; j < 4 * x + 4; j++)
    {
        for (i = 4 * y; i < 4 * y + 4; i++)
	    {
            // 只要后面2像素
            // �?
            if (mode == -2) {
                if (i == 4 * y + 2 || i == 4 * y + 3)
                {
                    draw_set_point(j, i + startPosY, 1);
                }
            }
            // 只要上面2像素
            // �?
            else if (mode == -3) {
                if (i == 4 * y + 0 || i == 4 * y + 1)
                {
                    draw_set_point(j, i + startPosY, 1);
                }
            }
            // 只要右面2像素
            // �?
            else if (mode == -4) {
                if (j == 4 * x + 2 || j == 4 * x + 3)
                {
                    draw_set_point(j, i + startPosY, 1);
                }
            }
            // 只要左面2像素
            // �?
            else if (mode == -5) {
                if (j == 4 * x + 0 || j == 4 * x + 1)
                {
                    draw_set_point(j, i + startPosY, 1);
                }
            }
		}
	}
}

/*
绘制食物
*/
void Paint_Food(int x, int y) 
{
	int i, j;
	for (i = 4 * y; i < 4 * y + 4; i++)
	{
		for (j = 4 * x; j < 4 * x + 4; j++)
		{
            // 只要中间2像素
			if (i == 4 * y + 1 || i == 4 * y + 2)
			{
				draw_set_point(j, i + startPosY, 1);
			}
			if (j == 4 * x + 1 || j == 4 * x + 2)
			{
				draw_set_point(j, i + startPosY, 1);
			}
		}
	}
}

// 绘制snake身体
void Paint_Body(int x, int y)
{
	int i, j;
	for (i = 4 * y; i < 4 * y + 4; i++)
	{
		for (j = 4 * x; j < 4 * x + 4; j++)
		{
            // 只要*..* 中间留空
			if (i == 4 * y || i == 4 * y + 3)
			{
				draw_set_point(j, i + startPosY, 1);
			}
			if (j == 4 * x || j == 4 * x + 3)
			{
				draw_set_point(j, i + startPosY, 1);
			}
		}
	}
}

// 清除�?
void Paint_Clean(int x, int y) 
{
	int i, j;
	for (i = 4 * y; i < 4 * y + 4; i++)
	{
		for (j = 4 * x; j < 4 * x + 4; j++)
		{
			draw_set_point(j, i + startPosY, 0);
		}
	}
}

// 创建地图
// 一次�? 四周围墙
void Creat_map()
{
	int i, j;
	
    for (j = 0; j < snakeWidth; j++)
    {
        for (i = 0; i < snakeHeight; i++)
	    {
            // 上下两行
			if (i == 0)
			{
				map[j][i] = -2;
			}
            if (i == 11) {
                map[j][i] = -3;
            }

            // 左右两列
			if (j == 0)
			{
				map[j][i] = -4;
			}
            if (j == 31) {
                map[j][i] = -5;
            }
		}
	}
}

void Snake_Init() // 蛇及食物初始�?
{
	int i;
    memset(map, 0, sizeof(map));
    memset(snake_Grid, 0, sizeof(snake_Grid));
	snakeLength = initSnakeLength; // 开始长度为5�?
	snakeScore = 0;
	snake_Grid[0][0] = 7; // x坐标,蛇头坐标
	snake_Grid[0][1] = 5; // y坐标
	for (i = 1; i < initSnakeLength; i++)
	{
		snake_Grid[i][0] = snake_Grid[0][0] - i;
		snake_Grid[i][1] = snake_Grid[0][1]; // 给刚开始的蛇身几个初始坐标
	}
	snakeDirection = RIGHT;
	Creat_map();

	Food();
}

/*
定时
Move();
Eat_Food();
drawSnake();
*/

/*
移动
1. 每移动一次就是清除尾�?
2. 是否吃了, 吃了长度变长
3. 后一点坐标变成前一点的坐标
4. 第一点根据方向变
---
 ---
*/
void Move()
{
	map[snake_Grid[snakeLength - 1][0]][snake_Grid[snakeLength - 1][1]] = 0; // 清除尾巴

	if (snakeEated) // 如果吃到了食�?
	{
		snakeLength++;
		snakeEated = false; // 设置为false，不然无限变�?
	}

	int i;
	for (i = snakeLength - 1; i > 0; i--) // 从尾巴开始，每一个点的位置等于它前面一个点的位�?
	{
		snake_Grid[i][0] = snake_Grid[i - 1][0];
		snake_Grid[i][1] = snake_Grid[i - 1][1];
	}

	switch (snakeDirection)
	{
	case UP:
		snake_Grid[0][1]--;
		break;
	case DOWN:
		snake_Grid[0][1]++;
		break;
	case LEFT:
		snake_Grid[0][0]--;
		break;
	case RIGHT:
		snake_Grid[0][0]++;
		break;
	}
	Grid_Bound(); // 坐标限定
}

// 坐标限定, 是否碰到墙了
void Grid_Bound()
{
    // 到最右了
	if (snake_Grid[0][0] == snakeWidth - 1)
		snake_Grid[0][0] = 1;
    // 到最左了
	else if (snake_Grid[0][0] == 0)
		snake_Grid[0][0] = snakeWidth - 2;
    // 到最底了
	else if (snake_Grid[0][1] == snakeHeight - 1)
		snake_Grid[0][1] = 1;
    // 到顶�?
	else if (snake_Grid[0][1] == 0)
		snake_Grid[0][1] = snakeHeight - 2;
}

int Chek(int i, int j) // 检查地图空�?
{
	if (map[i][j] != 0)
	{
		return 0;
	}
	return 1; // 是空位就返回1
}

// 生成食物
void Food()
{
	int i, j;
	do
	{
		i = rand() % 30; // 生成0~H-1之间的一个数
		j = rand() % 10;

	} while (Chek(i, j) == 0);
	map[i][j] = -1; // 画出食物
}

// 吃食�?
void Eat_Food()
{
	if (map[snake_Grid[0][0]][snake_Grid[0][1]] == -1) // 如果蛇头碰到食物，就重新投放食物，并且把食物点重置为0
	{
		snakeEated = true; // 标记已经吃到食物
		snakeScore += 1;
		Food();
		map[snake_Grid[0][0]][snake_Grid[0][1]] = 0; // 去掉食物
	}
}

// 画蛇
void drawSnake()
{
	int i, x, y;
	for (i = 0; i < snakeLength; i++)
	{
		x = snake_Grid[i][0];
		y = snake_Grid[i][1];
		map[x][y] = 2;
	}
}


bool GameOver() //游戏结束
{
	bool isGameOver = false;
	int sx = snake_Grid[0][0], sy = snake_Grid[0][1], i; //蛇头坐标
	for (i = 1; i < snakeLength; i++)												 //判断有没有吃到自�?
	{
		if (snake_Grid[i][0] == sx && snake_Grid[i][1] == sy)
			isGameOver = true;
	}
	return isGameOver;
}

// 游戏开�?
void game_snake_start()
{
    float temp;
    menu_close();

    srand(millis());
    display_setDrawFunc(game_snake_draw);
    buttons_setFuncs(btnRight, btnExit, btnLeft);

    Snake_Init();
}

// 游戏退�?
static bool btnExit()
{
    exitMeThenRun(display_load);
    return true;
}

// 向右移动 + 向下
static bool btnRight()
{
    if (snakeDirection == RIGHT || snakeDirection == LEFT) {
        snakeDirection = DOWN;
    } else {
        snakeDirection = RIGHT;
    }
    return true;
}

// 向左移动
static bool btnLeft()
{
    if (snakeDirection == LEFT || snakeDirection == RIGHT) {
        snakeDirection = UP;
    } else {
        snakeDirection = LEFT;
    }
    return true;
}

// 游戏绘图
display_t game_snake_draw()
{
    bool gameOver = GameOver();
    if (!gameOver) {
        if (renderI % 10 == 0) {
            Move();
            Eat_Food();
            drawSnake();
        }
    }
    Snake_Refresh();

    // 分数
    char scoreStr[12] = {""};
    sprintf(scoreStr, "%s%d", "Score:", snakeLength - initSnakeLength);
    draw_string_center(scoreStr, false, 0, FRAME_WIDTH, FRAME_HEIGHT - 8);

    if (gameOver) {
        draw_string_center("GAMEOVER!", false, 0, FRAME_WIDTH, (FRAME_HEIGHT-8)/2);
        //draw_string_center(scoreStr, false, 0, FRAME_WIDTH, (FRAME_HEIGHT-8)/2 + 10);
    }

    renderI++;
    renderI = renderI % 10;
    return DISPLAY_BUSY;  //返回屏幕刷新�?
}


#endif
