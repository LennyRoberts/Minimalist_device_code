#ifndef DATA_H
#define DATA_H

#include "global.h"

#define NO_SUCH_SENSER "nothing"
#define DEV_PATH "/dev/ttymxc5","/dev/ttymxc6","/dev/ttymxc7","/dev/ttymxc4","/dev/ttymxc3","/dev/ttymxc2","/dev/ttymxc1"
#define NUM	    (sizeof(dev) / sizeof(char*)) 
#define SPEED_9600	3

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


extern unsigned char buffer_result[ 256 ];
extern Control_T ctrolArray[ 16 ];

#endif