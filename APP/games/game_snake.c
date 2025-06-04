#include "common.h"

#if COMPILE_GAME_SNAKE

#define UP 3
#define DOWN 4
#define LEFT 1
#define RIGHT 2

// ��ͼ�ڵĵ�����
#define FOOD 1
#define BODY 2
#define BLANK 0

const byte startPosX = 2;    // ��ͼ����X ������Χǽ
const byte startPosY = 2;    // ��ͼ����Y ������Χǽ
const byte snakeWidth = 31;  // * 4 = 124 ������Χǽ
const byte snakeHeight = 13; // * 4 = 52 ������Χǽ

const byte wallSize = 1; // ǽ���, ǽ��map������

byte map[snakeWidth][snakeHeight] = {0}; // ��ͼ��С  x-y
int snake_Grid[128][2] = {0};            // ���������128����
const byte initSnakeLength = 3;
byte snakeLength;
byte snakeScore;
byte snakeDirection; // �ߵķ���
byte foodNum = 0;
bool snakeEated = false;
bool gameIsOver = false;
bool gameIsPaused = false;

byte refreshHz = 10; // ÿ10����Ⱦһ��, ֵԽС, �ٶ�Խ��
byte renderI = 0;

void Create_Wall(void);                    // ��ʼ����ͼ�߽�
void Move(void);                           // �ƶ�
void Paint_Map(byte x, byte y, byte mode); // ���Ƶ�ͼ
void Paint_Map2(void);
void Paint_Food(byte x, byte y);  // ����ʳ��
void Paint_Body(byte x, byte y);  // ��������
void drawSnake(void);             // ����
void Snake_Init(void);            // �߼�ʳ���ʼ��
void Paint_Clean(byte x, byte y); // �����
void Grid_Bound(void);            // �����޶�
bool GameOver(void);              // ��Ϸ����
void Gen_Food(void);              // ����ʳ��
void Eat_Food(void);              // ��ʳ��

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

-2 map ���ܿ�
-1 food
2 body
0 ����

ÿ�����Ӧ����Ļ��4x4����
Χǽ�� 4px ..**
body *..*
food .**.
*/

void Snake_Refresh() // ����ˢ��
{
    byte i, j, temp;

    for (i = 0; i < snakeWidth; i++)
    {
        for (j = 0; j < snakeHeight; j++)
        {
            temp = map[i][j];

            if (temp == BODY)
            {
                Paint_Body(i, j);
            }
            // else if (temp == -2 || temp == -3 || temp == -4 || temp == -5)
            // {
            //  Paint_Map(i, j, temp);
            // }
            else if (temp == FOOD)
            {
                Paint_Food(i, j);
            }
            else if (temp == BLANK)
            {
                Paint_Clean(i, j);
            }
        }
    }

    // Paint_Map2();

    // ����wall
    Create_Wall();
}

/*
����ʳ��
*/
void Paint_Food(byte x, byte y)
{
    byte i, j;

    for (i = 4 * y; i < 4 * y + 4; i++)
    {
        for (j = 4 * x; j < 4 * x + 4; j++)
        {
            // ֻҪ�м�2����
            if (i == 4 * y + 1 || i == 4 * y + 2)
            {
                draw_set_point(j + startPosX, i + startPosY, 1);
            }

            if (j == 4 * x + 1 || j == 4 * x + 2)
            {
                draw_set_point(j + startPosX, i + startPosY, 1);
            }
        }
    }
}

// ����snake����
void Paint_Body(byte x, byte y)
{
    byte i, j;

    for (i = 4 * y; i < 4 * y + 4; i++)
    {
        for (j = 4 * x; j < 4 * x + 4; j++)
        {
            // ֻҪ*..* �м�����
            if (i == 4 * y || i == 4 * y + 3)
            {
                draw_set_point(j + startPosX, i + startPosY, 1);
            }

            if (j == 4 * x || j == 4 * x + 3)
            {
                draw_set_point(j + startPosX, i + startPosY, 1);
            }
        }
    }
}

// �����
void Paint_Clean(byte x, byte y)
{
    byte i, j;

    for (i = 4 * y; i < 4 * y + 4; i++)
    {
        for (j = 4 * x; j < 4 * x + 4; j++)
        {
            draw_set_point(j + startPosX, i + startPosY, 0);
        }
    }
}

// ����Χǽ
// ֻ��1���ص�ǽ
void Create_Wall()
{
    byte leftX = startPosX - wallSize;
    byte topY = startPosY - wallSize;
    byte rightX = snakeWidth * 4 + wallSize;
    byte bottomY = snakeHeight * 4 + wallSize;

    // ��
    draw_fill_screen(leftX, topY, rightX, topY + wallSize - 1, 1);
    // ��
    draw_fill_screen(leftX, bottomY, rightX, bottomY + wallSize - 1, 1);

    // ��
    draw_fill_screen(leftX, topY, leftX + wallSize - 1, bottomY, 1);
    // ��
    draw_fill_screen(rightX, topY, rightX + wallSize - 1, bottomY, 1);
}

void Snake_Init() // �߼�ʳ���ʼ��
{
    memset(map, 0, sizeof(map));
    memset(snake_Grid, 0, sizeof(snake_Grid));
    snakeLength = initSnakeLength; // ��ʼ����Ϊ5��
    snakeScore = 0;
    foodNum = 0;
    gameIsOver = false;
    gameIsPaused = false;
    snake_Grid[0][0] = 7; // x����,��ͷ����
    snake_Grid[0][1] = 5; // y����

    for (byte i = 1; i < initSnakeLength; i++)
    {
        snake_Grid[i][0] = snake_Grid[0][0] - i;
        snake_Grid[i][1] = snake_Grid[0][1]; // ���տ�ʼ����������ʼ����
    }

    snakeDirection = RIGHT;

    // Ͷʳ
    Gen_Food();
}

/*
��ʱ
Move();
Eat_Food();
drawSnake();
*/

/*
�ƶ�
1. ÿ�ƶ�һ�ξ������β��
2. �Ƿ����, ���˳��ȱ䳤
3. ��һ��������ǰһ�������
4. ��һ����ݷ����
---
 ---
*/
void Move()
{
    map[snake_Grid[snakeLength - 1][0]][snake_Grid[snakeLength - 1][1]] = BLANK; // ���β��

    if (snakeEated)
    { // ����Ե���ʳ��
        snakeLength++;
        snakeEated = false; // ����Ϊfalse����Ȼ���ޱ䳤
    }

    byte i;

    for (i = snakeLength - 1; i > 0; i--)
    { // ��β�Ϳ�ʼ��ÿһ�����λ�õ�����ǰ��һ�����λ��
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

    Grid_Bound(); // �����޶�
}

// �����޶�, �Ƿ�����ǽ��
void Grid_Bound()
{
    // ��������
    if (snake_Grid[0][0] == snakeWidth)
    {
        snake_Grid[0][0] = 0;
    }
    // ��������
    else if (snake_Grid[0][0] == -1)
    {
        snake_Grid[0][0] = snakeWidth - 1;
    }
    // �������
    else if (snake_Grid[0][1] == snakeHeight)
    {
        snake_Grid[0][1] = 0;
    }
    // ������
    else if (snake_Grid[0][1] == -1)
    {
        snake_Grid[0][1] = snakeHeight - 1;
    }
}

bool isBlank(byte i, byte j) // ����ͼ��λ
{
    if (map[i][j] != 0)
    {
        return false;
    }

    return true; // �ǿ�λ�ͷ���1
}

void Gen_One_Food()
{
    byte i, j;

    do
    {
        i = rand() % snakeWidth; // ����0~H-1֮���һ����
        j = rand() % snakeHeight;
    } while (!isBlank(i, j));

    map[i][j] = FOOD; // ����ʳ��
    foodNum++;
}

// ����ʳ��
void Gen_Food()
{
    // �����, �����������
    if (foodNum > 0 && rand() % 2)
    {
        return;
    }

    Gen_One_Food();

    if (foodNum >= 5)
    {
        return;
    }

    // �������ʾһ��
    if (rand() % 2)
    {
        Gen_One_Food();
    }
}

// ��ʳ��
void Eat_Food()
{
    if (map[snake_Grid[0][0]][snake_Grid[0][1]] == FOOD)
    {                      // �����ͷ����ʳ�������Ͷ��ʳ����Ұ�ʳ�������Ϊ0
        snakeEated = true; // ����Ѿ��Ե�ʳ��
        snakeScore++;
        foodNum--;
        Gen_Food();
        map[snake_Grid[0][0]][snake_Grid[0][1]] = BLANK; // ȥ��ʳ��
    }
}

// ����
void drawSnake()
{
    byte i, x, y;

    for (i = 0; i < snakeLength; i++)
    {
        x = snake_Grid[i][0];
        y = snake_Grid[i][1];
        map[x][y] = BODY;
    }
}

bool GameOver() // ��Ϸ����
{
    bool isGameOver = false;
    byte sx = snake_Grid[0][0], sy = snake_Grid[0][1], i; // ��ͷ����

    for (i = 1; i < snakeLength; i++)
    { // �ж���û�гԵ��Լ�
        if (snake_Grid[i][0] == sx && snake_Grid[i][1] == sy)
        {
            isGameOver = true;
        }
    }

    return isGameOver;
}

// ��Ϸ��ʼ
void game_snake_start()
{
    menu_close();

    srand(millis());
    display_setDrawFunc(game_snake_draw);
    buttons_setFuncs(btnRight, btnExit, btnLeft);

    Snake_Init();
}

// ��Ϸ�˳�
static bool btnExit()
{
    // ��ûoverʱ, ���м�����ͣ
    if (!gameIsPaused && !gameIsOver)
    {
        gameIsPaused = true;
        return true;
    }

    exitMeThenRun(display_load);
    return true;
}

// �����ƶ� + ����
static bool btnRight()
{
    if (gameIsPaused)
    {
        gameIsPaused = false;
        return true;
    }

    if (gameIsOver)
    {
        Snake_Init();
        return true;
    }

    if (snakeDirection == RIGHT || snakeDirection == LEFT)
    {
        snakeDirection = DOWN;
    }
    else
    {
        snakeDirection = RIGHT;
    }

    return true;
}

// �����ƶ�
static bool btnLeft()
{
    if (gameIsPaused)
    {
        gameIsPaused = false;
        return true;
    }

    if (gameIsOver)
    {
        Snake_Init();
        return true;
    }

    if (snakeDirection == LEFT || snakeDirection == RIGHT)
    {
        snakeDirection = UP;
    }
    else
    {
        snakeDirection = LEFT;
    }

    return true;
}

// ��Ϸ��ͼ
display_t game_snake_draw()
{
    byte score = snakeLength - initSnakeLength;
    refreshHz = 10 - score / 5; // 5���ٶȱ��9, 10���ٶȱ��8

    if (refreshHz < 2)
    {
        refreshHz = 2;
    }

    gameIsOver = GameOver();

    if (!gameIsOver && !gameIsPaused)
    {
        if (renderI % refreshHz == 0)
        {
            Move();
            Eat_Food();
            drawSnake();
        }
    }

    Snake_Refresh();

    // ����
    char scoreStr[12] = {""};
    sprintf(scoreStr, "%s%d", "Score:", score);
    draw_string_center(scoreStr, false, 0, FRAME_WIDTH, FRAME_HEIGHT - 8);

    if (gameIsOver)
    {
        draw_string_center("GAME OVER!", false, 0, FRAME_WIDTH, (FRAME_HEIGHT - 8) / 2);
        // draw_string_center(scoreStr, false, 0, FRAME_WIDTH, (FRAME_HEIGHT-8)/2 + 10);
    }
    else if (gameIsPaused)
    {
        draw_string_center("GAME PAUSE!", false, 0, FRAME_WIDTH, (FRAME_HEIGHT - 8) / 2);
    }

    renderI++;
    renderI = renderI % refreshHz;
    return DISPLAY_BUSY; // ������Ļˢ��æ
}

// û��

/*
���Ƶ�ͼ 4px
  -2
-4  -5
  -3
void Paint_Map(byte x, byte y, byte mode)
{
    byte i, j;

    for (j = 4 * x; j < 4 * x + 4; j++)
    {
        for (i = 4 * y; i < 4 * y + 4; i++)
        {
            // ֻҪ����2����
            // ��
            if (mode == -2) {
                if (i == 4 * y + 2 || i == 4 * y + 3)
                {
                    draw_set_point(j + startPosX, i + startPosY, 1);
                }
            }
            // ֻҪ����2����
            // ��
            else if (mode == -3) {
                if (i == 4 * y + 0 || i == 4 * y + 1)
                {
                    draw_set_point(j + startPosX, i + startPosY, 1);
                }
            }
            // ֻҪ����2����
            // ��
            else if (mode == -4) {
                if (j == 4 * x + 2 || j == 4 * x + 3)
                {
                    draw_set_point(j + startPosX, i + startPosY, 1);
                }
            }
            // ֻҪ����2����
            // ��
            else if (mode == -5) {
                if (j == 4 * x + 0 || j == 4 * x + 1)
                {
                    draw_set_point(j + startPosX, i + startPosY, 1);
                }
            }
        }
    }
}

*/

#endif
