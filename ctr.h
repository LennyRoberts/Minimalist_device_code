#ifndef CTR_H
#define CTR_H

#include "global.h"

#define NOTOVERPROOF 0
#define HIT_RATE_OVERFLOW 0.8        //超标报警检测命中率
#define DURATION_DETECTION_S 30      //报警持续检测时间(s)，设置为持续时间相应秒数
#define ALARM_DISABLE_DURATION_S 600 //报警后禁用报警持续时间(s),设置为持续时间相应秒数

#define DANGER_ACTION_ENABLE_CHECK 4 //联动开关

#define DANGER_ACTION2_F_C 5         //强制关标志
#define DANGER_ACTION2_F_O 4         //强制开标志
#define DANGER_ACTION2_O_C 3         //普通关标志
#define DANGER_ACTION2_C_O 2         //普通开标志
#define DANGER_ACTION_STATUS_OPEN 1  //报警开
#define DANGER_ACTION_STATUS_CLOSE 0 //报警

#define DANGER_ACTION_OPEN1 1
#define DANGER_ACTION_OPEN2 3
#define DANGER_ACTION_CLOSE1 2
#define DANGER_ACTION_CLOSE2 4
#define DANGER_ACTION_STATUS1 5
#define DANGER_ACTION_STATUS2 6

#define STYLE_1 0
#define STYLE_2 1

#define TIME_ALARM 3



extern int count_for_danger_action;
extern System_T systemParam;
extern int period;

int manageDangerAction(int i);
extern void openGPIO(unsigned char sel);
extern void initActionStatus();
#endif