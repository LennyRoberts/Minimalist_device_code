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

typedef struct
{
	uint16_t auth;				//授权信息
	uint16_t upTime;			//上传时间
	uint8_t heartbeatTime;		//心跳包
	uint8_t pageTime;			//换页时长
	uint16_t ledTime;			//屏幕更新时间
	uint8_t testT;				//读取模式 
	uint8_t actionType;			//单双联动状态
	uint16_t enable;			//功能位 -
	uint8_t ip[4];				//控制器IP
	uint8_t netMasks[4];		//子网掩码
	uint8_t gateWay[4];			//网关
	uint8_t	nTotal;				//设备总数
	uint8_t	n;					//同步设备编号
	uint8_t	version;			//版本号
	uint8_t	lenValueLed;		// first bit(led align)+ led font size +  for led space for senser's value
	uint16_t timeWeather;
	uint16_t ledBlankConfig;	// row space; first blank; second blank; third black;
	uint32_t weatherConf;		// led weather size and color, effect, speed;
	uint32_t timeZero;			// time zero 
	uint8_t backValue;			// time zero 
	uint8_t growth;				// 扬尘bound限制 
	uint16_t n2;				// nothing 
//	uint32_t enable2;			// enable2
	uint16_t enable2;			// enable2
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

extern unsigned char buffer_result[ 256 ];
extern Control_T ctrolArray[ 16 ];
#endif