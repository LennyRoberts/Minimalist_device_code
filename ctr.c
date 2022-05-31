#include "ctr.h"

unsigned char danger_channel;

float avg_overflow = 0.0; //触发报警的平均检测值
int alarm_flag_n = 0;     //启动报警记录 0:规定时间内无报警有报警;1:规定时间内有报警有报警
/*检查噪声值是否超标*/
uint8_t checkNoiseAlarmValue(int i)
{
    int result = NOTOVERPROOF;
    float threshole_value = 0.0;
    float value = 0.0;
    static time_t start_time = 0;
    time_t end_time = time(NULL); //局部变量：每次取当前时间
    static int normal_counter = 1;
    static int overflow_counter = 1;
    static int first_overflow_flag = 1; // 1:首次超标
    threshole_value = getStartValue(i);
    value = getSenserValue(i);

    if (value >= threshole_value) {
        if (first_overflow_flag == 1 && alarm_flag_n == 0) { //首次超标且无报警
            start_time = time(NULL);
            first_overflow_flag = 0;
            overflow_counter = 1;
            avg_overflow = value;
            normal_counter = 1;
            result = NOTOVERPROOF;
        }
        else{
            overflow_counter++;
            printStyle("overflow=%d\n", overflow_counter);
        }
    } 
    else
        normal_counter++;
    end_time = time(NULL);
    if ((end_time - start_time) > 
        (ALARM_DISABLE_DURATION_S + DURATION_DETECTION_S)){ //超过报警禁用时间后解禁
        if (alarm_flag_n == 1)
            alarm_flag_n = 0;
    }
    else if((end_time - start_time) >= DURATION_DETECTION_S){ //超标报警持续检测时间
        if(alarm_flag_n == 0){
            float hit_rate = (float)overflow_counter / (overflow_counter + normal_counter); //计算命中率
            if (hit_rate >= HIT_RATE_OVERFLOW){
                alarm_flag_n = 1;
                result = i;
            }
            else{
                alarm_flag_n = 0;
                result = NOTOVERPROOF;
            }
            first_overflow_flag = 1;
        }
    }
    return result;
}

uint32_t checkEnable(uint32_t check)
{
    return (systemParam.enable & check);
}

uint16_t getWaitTime(int n)
{
    n = n - 1;
    return ctrolArray[n].waitTime; //延迟时间
}

int actionForDanger(int select)
{
    int result;
    unsigned int i;
    switch (select){
        case DANGER_ACTION_OPEN1:
            openGPIO(GPIO1);
            result = DANGER_ACTION_STATUS_OPEN;
            break;
        case DANGER_ACTION_CLOSE1:
            closeGPIO(GPIO1);
            result = DANGER_ACTION_STATUS_CLOSE;
            break;
        case DANGER_ACTION_OPEN2:
            openGPIO(GPIO2);
            openGPIO(GPIO3);
            result = DANGER_ACTION_STATUS_OPEN;
            break;
        case DANGER_ACTION_CLOSE2:
            closeGPIO(GPIO2);
            closeGPIO(GPIO3);
            result = DANGER_ACTION_STATUS_CLOSE;
            break;
        default:
            break;
    }
    return result;
}

int EntranceForDangerAction(int style, int value, int warmingValue, int dangerValue)
{
    int len;
    int status;
    int result;
    len = 0;
    result = 0;
    if (style == STYLE_1 && checkEnable(DANGER_ACTION_ENABLE_CHECK)) {
        status = actionForDanger(DANGER_ACTION_STATUS1);
        result = status;
        if (value < warmingValue && status != DANGER_ACTION_STATUS_CLOSE){
            actionForDanger(DANGER_ACTION_CLOSE1);
            result = DANGER_ACTION_STATUS_CLOSE;
        }
        else if (value >= warmingValue && value < dangerValue && status != DANGER_ACTION_STATUS_OPEN) {
            actionForDanger(DANGER_ACTION_OPEN1);
            result = DANGER_ACTION_STATUS_OPEN;
        }
        else if (value >= dangerValue && status != DANGER_ACTION_STATUS_OPEN){
            actionForDanger(DANGER_ACTION_OPEN1);
            result = DANGER_ACTION_STATUS_OPEN;
        }
    }
    else if (style == STYLE_2 && checkEnable(DANGER_ACTION_ENABLE_CHECK)){
        status = actionForDanger(DANGER_ACTION_STATUS2);
        result = status;
        if (value < warmingValue && status != DANGER_ACTION_STATUS_CLOSE){
            actionForDanger(DANGER_ACTION_CLOSE2);
            result = DANGER_ACTION_STATUS_CLOSE;
        }
        else if (value >= warmingValue && value < dangerValue && status != DANGER_ACTION_STATUS_OPEN) {
            actionForDanger(DANGER_ACTION_OPEN2);
            result = DANGER_ACTION_STATUS_OPEN;
        }
        else if (value >= dangerValue && status != DANGER_ACTION_STATUS_OPEN){
            actionForDanger(DANGER_ACTION_OPEN2);
            result = DANGER_ACTION_STATUS_OPEN;
        }
    }
    return result;
}

/*设备联动管理程序*/
int manageDangerAction(int i)
{
    int temp;      //检测周期
    int result;    //状态(强制？不强制？超标？不超标？)
    int choise;    //单开联动 || 多开联动
    int index;     //检查所有参数返回的值：NOTOVERPROOF表示不超标，其他表示超标的参数
    uint8_t onoff; //运行第1步时，onoff记录开/关
    static short time_adjust = 0;
    static time_t beforeti;
    static time_t currtime;
    long timediff = 0;
    result = FALSE;
    if (!checkEnable(DANGER_ACTION_ENABLE_CHECK))
    { //联动总开关 period是否超时 参数非法？？
        printStr("danger anction not enable.");
        return result;
    }
    temp = getPeriod(danger_channel); //检测周期
    if (period < temp)
    { //休眠
        printStyle("-----------------period value = %d\n", period);
        printStyle("-----------------temp value = %d\n", temp);
        period += TIME_ALARM;
    }
    else{
        choise = choiseDanger(); // true:2路open && 单开联动  false:2路close or 双开联动
        period = temp; //超时后保持period为周期时间，为了下次循环的时候直接进入检测，不用等待period的时间再检测。除非后面period清零了
        // 1、判断是否需要开启或者关闭联动
        if (STATUS_ACTION_INIT != checkActionStatus())
        { //是否有强制
            if (STATUS_ACTION_ENABLE == checkActionStatus())
            {
                printStr("强制开force open");
                onoff = DANGER_ACTION_STATUS_OPEN; //开
                result = DANGER_ACTION2_F_O;
            }
            else if (STATUS_ACTION_DISABLE == checkActionStatus())
            {
                printStr("强制关force close");
                onoff = DANGER_ACTION_STATUS_CLOSE; //关。
                result = DANGER_ACTION2_F_C;
            }
        }
        else{ //没有强制
            index = checkNoiseAlarmValue(i);
            if (NOTOVERPROOF == index){ //没有超标
                result = FALSE;
                onoff = DANGER_ACTION_STATUS_CLOSE; //关
            }
            else { //有超标
                result = TRUE;
                danger_channel = index; //超标参数下标
                onoff = DANGER_ACTION_STATUS_OPEN; //开
            }
        }

        // 2、处理联动，开启或者关闭联动
        if (onoff == DANGER_ACTION_STATUS_OPEN && 
            count_for_danger_action <= getMaxTime(danger_channel)){ //报警开&&报警持续时长未够
            if (count_for_danger_action >= getWaitTime(danger_channel)){ //是否超过2路等待时间
                if (DANGER_ACTION_STATUS_OPEN != actionForDanger(DANGER_ACTION_STATUS2))
                {                                     // 2路未开启联动
                    if (result != DANGER_ACTION2_F_O) //非强制开
                        result = DANGER_ACTION2_C_O;  //普通开
                }
                EntranceForDangerAction(STYLE_2, 0, 0, 0); //开报警2
                if (TRUE == choise){// true: 表示选择第一种联动方式（开启报警2，关闭报警1） 单开联动
                    EntranceForDangerAction(STYLE_1, 0, 1, 1); //关报警1
                    printStr("-----------------action type is 1,need close action1\n");
                }
                else if (0 == count_for_danger_action)         // 0 equal waiting time;
                    EntranceForDangerAction(STYLE_1, 0, 0, 0); //开报警1
                printStr("-----------------action type is 2, no need close action1\n");
            }
            else{
                result = TRUE; // clear F_O, cause it is not the time
                EntranceForDangerAction(STYLE_1, 0, 0, 0); //开报警1
                printStr("-----------------open action1\n");
            }
            count_for_danger_action += TIME_ALARM;
        }
        else
        { //关
            count_for_danger_action = 0;
            if (DANGER_ACTION_STATUS_OPEN == actionForDanger(DANGER_ACTION_STATUS2))
            {
                if (result == DANGER_ACTION2_F_O || result == DANGER_ACTION2_F_C) //强制开或关
                    result = DANGER_ACTION2_F_C; //强制关
                else
                {
                    result = DANGER_ACTION2_O_C; //普通关
                    period = 0;
                }
            } //处于关闭状态-关闭，这个时候不需要清零，清零的话下次检测又要间隔period时间段了
            EntranceForDangerAction(STYLE_1, 0, 1, 1); //关闭报警1
            EntranceForDangerAction(STYLE_2, 0, 1, 1); //关闭报警2
            initActionStatus();                        //初始化联动状态(非强制)
        }
    }
    return result;
}