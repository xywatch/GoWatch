#include "common.h"
extern bool keep_on;

stepLogData_s stepLogs[STEP_LOG_COUNT]; // 0 ������, 1��ǰ��

static display_t draw()
{
  char p[6], i;
  draw_string("Date   Step", NOINVERT, 0, 0);

  for (i = 0; i < STEP_LOG_COUNT; i++)
  {
    sprintf((char *)p, "%02d-%02d", stepLogs[i].month + 1, stepLogs[i].date); // time
    draw_string(p, NOINVERT, 0, 9 + i * 9);
    sprintf((char *)p, "%d", stepLogs[i].stepCount);
    draw_string(p, NOINVERT, 50, 9 + i * 9);
  }

  return DISPLAY_DONE;
}

static bool btnExit()
{
  keep_on = false;
  back_to_watchface();
  return true;
}

void addStepLog(void)
{
  byte year = timeDate.date.year;         // ��2000Ϊ��׼
  byte month = (byte)timeDate.date.month; // 0-11, �·�, 0:һ��, 11:ʮ����
  byte date = timeDate.date.date;         // 1-31, ����

  // ������һ��
  if (date == 1)
  {
    // ǰһ����
    if (month == 0)
    {
      // ǰһ�����һ��
      year--;
      month = 11; // ʮ����
      date = 31;  // ʮ���¹̶�31��
    }
    else
    {
      // ��һ����
      month--;
      // �����ϸ����ж�����
      date = time_monthDayCount((month_t)month, year);
    }
  }
  else
  {
    // ǰһ��
    date--;
  }

  // ���ݺ��� 0->1
  // ʹ��memmove����memcpy���԰�ȫ�����ص����ڴ�����
  memmove(&stepLogs[1], &stepLogs[0], (STEP_LOG_COUNT-1) * sizeof(stepLogData_s));

  stepLogs[0].year = year;
  stepLogs[0].month = (month_t)month;
  stepLogs[0].date = date;
  stepLogs[0].stepCount = bma_getStepCount();

  // appconfig_save_step_log();
}

void showStepLog(void)
{
  display_setDrawFunc(draw);
  buttons_setFuncs(btnExit, btnExit, btnExit);
  showMeThenRun(NULL);
  keep_on = true;
}