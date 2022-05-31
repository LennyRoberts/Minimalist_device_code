#ifndef DATA_H
#define DATA_H

#include "global.h"

#define NO_SUCH_SENSER "nothing"
#define DEV_PATH "/dev/ttymxc5","/dev/ttymxc6","/dev/ttymxc7","/dev/ttymxc4","/dev/ttymxc3","/dev/ttymxc2","/dev/ttymxc1"
#define NUM	    (sizeof(dev) / sizeof(char*)) 
#define SPEED_9600	3

#define AVER 1
#define MIN 2
#define MAX 3
#define SUM 4

#define READ_BY_LEN 0
#define READ_BY_JSON 1
#define LEN_MODBUS_CRC 2

#define SERIAL_232_0 0
#define SERIAL_232_1 1
#define SERIAL_232_2 2
#define SERIAL_485_2 3
#define SERIAL_485_1 4

#define SMOKE  1
#define RAINALL 2
#define TEMPERATURE 3
#define HUMIDITY 4
#define WIND_SPEED 5
#define WIND_DIRECTION 6

typedef struct
{
	char name_en[ 16 ];//中文名字
	char name[ 16 ];//英文名字
	char unit[ 8 ];
	unsigned char lenCmd;
	const unsigned char* cmd;
	unsigned char len;
	unsigned char lenRes;
	unsigned char lenSrc;
	unsigned char srcOffset;
	unsigned char desOffset;
	float divide;
}ParaT;

typedef struct AverageT
{
	char status;		// D:故障;B:通讯异常;F:停运;
	int realvalue;		//实时值(可能被放大了)
	float divide;		//放大倍数
	float fl_realvalue; //实时值-浮点型

	int real_min;
	int real_max;
	long long real_sum;
	int real_count;

	int min_min;
	int min_max;
	long long min_sum;
	int min_count;

	int hour_min;
	int hour_max;
	long long hour_sum;
	int hour_count;

	int day_min;
	int day_max;
	long long day_sum;
	int day_count;
} averagetT;

int getResult(int n);
char *GetParaName(int i);
char *GetParaLabel(int i);
float getSenserValue(int n);
int checkFdStatus(int *fdArray, int n, unsigned char isRead, int nt);
float getDataAverMinMaxSum(int n, int date, int class);
extern unsigned char buffer_result[256];
extern Control_T ctrolArray[ 16 ];
extern void ClearData(int n);
#endif