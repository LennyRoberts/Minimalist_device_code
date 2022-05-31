#include "data.h"

const char *dev[] = {DEV_PATH};
int fdArray[ NUM ]; //存放串口fd
int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	               B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,
	              38400,  19200,  9600, 4800, 2400, 1200,  300, };

/*传感器读取指令*/
const unsigned char tH_Cmd[] = {0x01, 0x04, 0x00, 0x01, 0x00, 0x05, 0x61, 0xC9};
const unsigned char smoke_Cmd[] = {0x01, 0x04, 0x00, 0x01, 0x00, 0x05, 0x61, 0xC9};
const unsigned char rainall_Cmd[] = {0x07, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x6C};
const unsigned char windSpeedCMD[] = {0x05, 0x03, 0x00, 0x00, 0x00, 0x01, 0x85, 0x8E};
const unsigned char windDirectionCMD[] = {0x06, 0x03, 0x00, 0x00, 0x00, 0x01, 0x85, 0xBD};

/*响应指令解析信息*/
const ParaT para[] = {
    {"NO_SUCH_SENSER", "NO_SUCH_SENSER", "", 0, NULL, 0, 0, 0, 0, 0, 0},
    {"smoke", "烟雾", "ug/m3", sizeof(smoke_Cmd), smoke_Cmd, 4, 15, 2, 3, 2, 1},
    {"rain", "雨量", "mm", sizeof(rainall_Cmd), rainall_Cmd, 4, 7, 2, 3, 46, 10},
    {"T", "温度", "℃", sizeof(tH_Cmd), tH_Cmd, 4, 15, 2, 7, 26, 100},
    {"H", "湿度", "%RH", sizeof(tH_Cmd), tH_Cmd, 4, 15, 2, 9, 30, 100},
    {"WS", "风速", "m/s", sizeof(windSpeedCMD), windSpeedCMD, 4, 7, 2, 3, 18, 10},
    {"WD", "风向", "", sizeof(windDirectionCMD),windDirectionCMD, 4, 7, 2, 3, 22, 1},
};

uint8_t getSenserNum(){return  sizeof(para)/sizeof(ParaT);}
int getMin(int n) {n = n -1;return ctrolArray[ n ].min;}
void initResult()
{
	int result, i;
	memset(buffer_result, 0, sizeof(buffer_result));
	result = getSenserNum();
	for(i = 1; i < result; i++){
		setValue(&buffer_result[ para[i].desOffset], getMin(i)*para[i].divide, para[ i ].len);
	}
}

void set_speed(int fd, int speed)
{
	int   i;
	int   status;
	struct termios Opt;
	tcgetattr(fd, &Opt);//用来获取终端参数。

	for( i= 0;  i < (int)NUM;  i++){
		if (speed == name_arr[i]){
			tcflush(fd, TCIOFLUSH);//tcflush函数刷清输入缓存或输出缓
			cfsetispeed(&Opt, speed_arr[i]);//需要设置输入的波特类
			cfsetospeed(&Opt, speed_arr[i]);//需要设置输出的波特类
			status = tcsetattr(fd, TCSANOW, &Opt);//tcsetattr是用于设置终端参数的函数
			if(status != 0){
				printStr("error set speed\n");
				perror("tcsetattr fd1");
			}
			printStyle("baud result:%d\n", status);
			return;
		}
		tcflush(fd,TCIOFLUSH);
	}
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;

	if( tcgetattr( fd,&options) !=  0){
		perror("SetupSerial 1");
		return(FALSE);
	}
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//not deal enter
	options.c_oflag &= ~OPOST;
	options.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);
	options.c_cflag &= ~CSIZE;

	switch (databits) {
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return (FALSE);
	}

	switch (parity){
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */
			options.c_iflag &= ~(ICRNL|IGNCR);
			options.c_lflag &= ~(ICANON );
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB);   
			options.c_iflag |= INPCK;             
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;   /* */  
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
	}
	switch (stopbits){
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
	}

	/* Set input parity option */
	if (parity != 'N')
		options.c_iflag |= INPCK;
	options.c_cc[VTIME] = 150; // 15 seconds
	options.c_cc[VMIN] = 0;
	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
	if(tcsetattr(fd,TCSANOW,&options) != 0){
		perror("SetupSerial 3");
		return (FALSE);
	}
	return (TRUE);
}

int open232(const char* device, int* n)
{
	int result, fd;
	result = 0;
	fd = open(device, O_RDWR | O_NOCTTY);
	if(fd > 0) set_speed(fd, name_arr[ SPEED_9600 ]); //波特率9600
	else result =  2;
	*n = fd;
	if(set_Parity(fd, 8, 1,'N') == FALSE) result =  1;
	if(result == 0) *n = fd;
	return result;
}

int openAllSensers(void)
{
	int i;
	int result;
	int num = NUM;
	for (i = 0; i < num; i++){
		result = open232(dev[ i ], &fdArray[ i]);
		if (result != 0)
            break;
	}
	if(i != num) result = 1;
	else result = 0;
	return result;
}