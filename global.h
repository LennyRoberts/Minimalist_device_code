#ifndef GLOBAL_H
#define GLOBAL_H
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <endian.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <mcheck.h>
#include <string.h>     
#include <unistd.h>     
#include <termios.h>        
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <stdbool.h>
#include <math.h>

#define FALSE -1
#define TRUE 1

#define LINE_AUTH_CONFIG 	0
#define LINE_TIME_CONFIG 	0
#define LINE_IP_CONFIG 		1
#define LINE_CONTROL_CONFIG 2

#define STATUS_ACTION_INIT		    1 //强制使能
#define STATUS_ACTION_ENABLE		2 //强制开
#define STATUS_ACTION_DISABLE		4 //强制关

#define TIMEOUT_WATCHDOG 60
#define STATUS_WD_INIT 1
#define STATUS_WD_CLOSE 2

#define INVALID_FD -1
#define TIME_S_FD_NOT_AVAILABLE 0	 /*s*/
#define TIME_FD_NOT_AVAILABLE 500000 /*us*/

#define ONCE 5
#define REAL 4
#define DAY 3
#define HOUR 2
#define MINUTE 1

#define GPIO1 1
#define GPIO2 2
#define GPIO3 3
#define GPIO4 4

typedef struct
{
	uint16_t auth;				//授权信息
	uint16_t upTime;			//上传时间
	uint8_t heartbeatTime;		//心跳包
	uint8_t testT;				//读取模式 
	uint8_t actionType;			//单双联动状态
	uint16_t enable;			//功能位 -
	uint8_t ip[4];				//控制器IP
	uint8_t netMasks[4];		//子网掩码
	uint8_t gateWay[4];			//网关
	uint8_t	nTotal;				//设备总数
	uint8_t	n;					//同步设备编号
	uint8_t	version;			//版本号
}System_T;

typedef struct 
{
	uint8_t destIp[4];
	uint16_t destPort;
}IP_T;

typedef struct ControlStruct
{
	int min;
	int max;
	int correcting;
	uint8_t isOpen;
	uint8_t isHigh;
	uint16_t period;
	uint16_t waitTime;
	uint16_t maxTime;
	int startValue;
	int endValue;
}Control_T;

void initGPIO( );
int commandExecute(char *cmd, char *isRead);
int checkFdStatus(int *fdArray, int n, unsigned char isRead, int nt);
void openGPIO(unsigned char sel);
void initActionStatus();
uint16_t getPeriod(int n);
int ipToString(char *p, uint8_t n);
uint16_t getDestPort(uint8_t n);

extern unsigned char buffer_result[256];
extern Control_T ctrolArray[16];
extern time_t time_real_Ago; //实时数据上传时间查看
extern time_t time_real_Now;
extern unsigned char ID[50];
extern System_T systemParam;
extern int period;
extern int count_for_danger_action;
extern unsigned char danger_channel;
#endif