#include "global.h"

Control_T ctrolArray[ 16 ];
System_T  systemParam;
IP_T      destIpArray[ 9 ];
static FILE *file;
const unsigned char *directionZH[ ] = { "东北偏北", "东北", "东北偏东", "正东", "东南偏东", "东南", "东南偏南", "正南", "西南偏南", "西南", "西南偏西", "正西", "西北偏西", "西北", "西北偏北", "正北" };
int count_for_danger_action;          //持续/等待时长
int action_status;                     //暂知0-2位为联动状态,
unsigned char buffer_result[ 256 ];

//命令封装
int commandExecute(char *cmd, char *isRead) 
{
	FILE *fp;
	int result;

	fp = popen(cmd, isRead);	//创建一个管道写入
	if(fp == NULL){
		printf("fp ==	null\n");
		result = FALSE;
	}
	else{
		pclose(fp);
		result = TRUE;
	}

	return result;
}

void initGPIO( )
{
	commandExecute("echo 134 > /sys/class/gpio/export", "w"); //GPIO0  (5_6) 导出引脚编号为134的GPIO
	commandExecute("echo 133 > /sys/class/gpio/export", "w"); //GPIO1  (5_5)
	commandExecute("echo 137 > /sys/class/gpio/export", "w"); //GPIO2  (5_9)
	commandExecute("echo 120 > /sys/class/gpio/export", "w"); //gpio3  (4_24)

	commandExecute("echo out > /sys/class/gpio/gpio134/direction", "w"); //设置GPIO134方向为输出
	commandExecute("echo out > /sys/class/gpio/gpio133/direction", "w");
	commandExecute("echo out > /sys/class/gpio/gpio137/direction", "w");
	commandExecute("echo out > /sys/class/gpio/gpio120/direction", "w");

	commandExecute("echo 0 > /sys/class/gpio/gpio134/value", "w"); //定义GPIO134端口值为0，输出低电平
	commandExecute("echo 0 > /sys/class/gpio/gpio133/value", "w");
	commandExecute("echo 0 > /sys/class/gpio/gpio137/value", "w");
	commandExecute("echo 0 > /sys/class/gpio/gpio120/value", "w");
}

uint8_t* getDestIp(uint8_t n) {return destIpArray[n].destIp;}
uint8_t* getSystemIP() {return systemParam.ip;}
uint16_t getDestPort(uint8_t n) {return destIpArray[n].destPort;}
uint8_t* getNetMasks() {return systemParam.netMasks;}
uint8_t* getGateWay() {return systemParam.gateWay;}
void setDestPort(uint8_t i, uint16_t n) {destIpArray[i].destPort = n;}
void rebootSystem() { commandExecute("reboot -f", "r"); } //重启系统
void openConfigFile() {file = fopen("/home/app/config.txt", "rt+");}
void closeConfigFile() {fflush(file);fclose(file);}



unsigned char char2hex(unsigned char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	else if (c >= 'A' && c <= 'Z') return c - 'A' + 0x0A;
	else if (c >= 'a' && c <= 'z') return c - 'a' + 0x0A;
	else return 0;
}

int changeToHex(unsigned char* src, unsigned char* des)
{
	int i, len;
	unsigned char h, l;
	len = strlen(src) / 2;
	for(i = 0; i < len; i++){
		h = char2hex(*(src + i * 2));
		l = char2hex(*(src + i * 2 + 1));
		*(des + i) = ((h << 4) + l);
	}
	return len;
}

//设置网络
void setNet( ) 
{
	FILE *file1;
	int result;
	char cmd[ 38 ];
	char addDefultRouteCmd[ 40 ] = { 0 };
	printf("\nset net!\n");
	memset(cmd, 'a', 38);
	file1 = fopen("/etc/resolv.conf", "a+");
	if(file1 != NULL){
		printf("set nameserver success!\n");
		sprintf(&cmd[ 0 ], "nameserver %d.%d.%d.%d\n", systemParam.gateWay[ 0 ], systemParam.gateWay[ 1 ], 
		        systemParam.gateWay[ 2 ], systemParam.gateWay[ 3 ]);
		fprintf(file1, "%s\n", cmd);
		fprintf(file1, "%s", "nameserver 114.114.114.114");
		fclose(file1);
	}
	else{
		printf("set nameserver error!\n");
		rebootSystem( );
	}

	/* 这里将IP和掩码等用管道封装成外部命令，然后通过写命令来设置网络配置，本次暂时性的设置 */
	sprintf(&cmd[ 0 ], "ifconfig eth0 %d.%d.%d.%d", systemParam.ip[ 0 ], 
	        systemParam.ip[ 1 ], systemParam.ip[ 2 ], systemParam.ip[ 3 ]);
	printfs("%s\n", cmd);
	commandExecute(cmd, "w");
	sprintf(&cmd[ 0 ], "ifconfig eth0 netmask %d.%d.%d.%d", systemParam.netMasks[ 0 ], systemParam.netMasks[ 1 ], 
	        systemParam.netMasks[ 2 ], systemParam.netMasks[ 3 ]);
	printf(cmd);
	commandExecute(cmd, "w");
	sprintf(&cmd[ 0 ], "route add default gw %d.%d.%d.%d", systemParam.gateWay[ 0 ], systemParam.gateWay[ 1 ], 
	        systemParam.gateWay[ 2 ], systemParam.gateWay[ 3 ]);
	printf(cmd);
	result = commandExecute(cmd, "w");
	if(result == FALSE){
		printf("set gw ,fp ==	null\n");
		rebootSystem( );
	}
	commandExecute("ifconfig lo up", "w");
}

void getSystemParam( )
{
	int i;
	unsigned char *p;
	openConfigFile( );					//打开config文件
	p = (unsigned char *)(&systemParam);
	getN_InFile(p, LINE_AUTH_CONFIG);	//获取config第一行设备配置参数
	p = (unsigned char *)(&destIpArray);
	getN_InFile(p, LINE_IP_CONFIG);		//获取config第二行网络配置参数
	closeConfigFile( );
}

int getN_InFile(unsigned char *des, int n)
{
	int len = 0;
	int i = 0;
	char StrLine[ 2048 ];

	if(file == NULL){
		printf("getNFile file open error\n");
		return FALSE;
	}
	else{
		printf("getN_InFile rewind\n");
		fseek(file, 0L, SEEK_SET);
	}
	while(i <= n){
		memset(StrLine, 0, 2048);
		fgets(StrLine, 2048, file);
		i += 1;
	}
	if(i != 0 && des != NULL){
		len = changeToHex(StrLine, des);
		printf("to see the data read from txt\n");
	}
	return len;
}


void getControl( )
{
	int i;
	int len;
	openConfigFile( );	//打开config文件
	printf("size of control_t=%d\n", sizeof(Control_T));
	len = sizeof(ctrolArray) / sizeof(Control_T);
	for(i = 0; i < len; i++)
		getN_InFile((unsigned char *)(&ctrolArray[ i ]), i + LINE_CONTROL_CONFIG);
	closeConfigFile( );	//关闭config文件
}

void initSystem( )
{
	printf("\nInitSystem Start:\n");
	memset(ctrolArray, 0, sizeof(ctrolArray)); //清零缓存区
	getControl( );		//打开config文件并关闭
	getSystemParam( );	//打印系统信息，即config.txt中定义的设备配置的系统信息和网络配置信息
	setNet( );			//设置网络，将config.txt中的网络配置设置到设备系统当中
	printf("\nInit System Finished!\n\n");
}

void initActionStatus()
{
	action_status = STATUS_ACTION_INIT;
}
