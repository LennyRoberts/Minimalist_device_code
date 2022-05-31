#include "data.h"

averagetT averRecArr[sizeof(para) / sizeof(para[0])] = {0};
const char *dev[] = {DEV_PATH};
int fdArray[ NUM ]; //存放串口fd
int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
	               B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 38400,  19200,  9600,  4800,  2400,  1200,  300,
	              38400,  19200,  9600, 4800, 2400, 1200,  300, };
unsigned char buffer[200];

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

void (*func[])(void) = {GetSmokeValue,GetRainValue,GetTemperatureValue,
                        GetHumidtyValue,GetWindSpeedValue,GetWindDirectionValue};

uint8_t getSenserNum() {return sizeof(para) / sizeof(ParaT);}
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

void clearAverage(int date, averagetT *str, int sum_senser)
{
	averagetT *Decv = str;
	int n;
	int temp = date;
	if (2 == date){
		for(n = 0; n < sum_senser; n++){
			Decv[n].day_sum = 0;
			Decv[n].day_count = 0;
			Decv[n].day_min = 0;
			Decv[n].day_max = 0;
		}
		date--;
	}
	if(1 == date){
		for(n = 0; n < sum_senser; n++){
			Decv[n].hour_sum = 0;
			Decv[n].hour_count = 0;
			Decv[n].hour_min = 0;
			Decv[n].hour_max = 0;
		}
		date--;
	}
	if(0 == date){
		for(n = 0; n < sum_senser; n++){
			Decv[n].min_sum = 0;
			Decv[n].min_count = 0;
			Decv[n].min_min = 0;
			Decv[n].min_max = 0;
		}
		date--;
	}
}

void ClearData(int n)
{
	clearAverage(n, averRecArr, getSenserNum());
}

float getSenserValue(int n)
{
	float result = 0.0;
	ParaT paraTemp;
	paraTemp = para[n];
	if (strcmp(paraTemp.name, NO_SUCH_SENSER) == 0){ //无传感器就不需要继续了
		printf("[%d:%s]", n, paraTemp.name);
		return -1;
	}else
		printf("\n%d : %s :", n, paraTemp.name);

	if (averRecArr[n - 1].real_count != 0 && paraTemp.divide != 0)
		result = ((float)averRecArr[n - 1].real_sum) / (averRecArr[n - 1].real_count) / paraTemp.divide;
	else
		result = getRealValue(&buffer_result[paraTemp.desOffset], paraTemp.len, paraTemp.divide);

	return result;
}

char *GetParaName(int i)
{
	return para[i].name;
}

char *GetParaLabel(int i)
{
	return para[i].name_en;
}

float getDataAverMinMaxSum(int n, int date, int class)
{
	averagetT *Decv = averRecArr;
	float temp_value = 0.0;
	float divide = 0;
	if (divide < 0.1 && divide > -0.1)
		divide = para[n].divide;
	n--;
	if (REAL == date){
		if (Decv[n].real_count)
			return ((float)Decv[n].real_sum / Decv[n].real_count / divide);
		else
			return ((float)Decv[n].realvalue / divide);
	}
	if (MINUTE == date){
		if (Decv[n].min_count){
			if (class == AVER)
				return ((float)Decv[n].min_sum / Decv[n].min_count / divide);
			else if (class == MIN)
				return ((float)Decv[n].min_min / divide);
			else if (class == MAX)
				return ((float)Decv[n].min_max / divide);
			else if (class == SUM)
				return ((float)Decv[n].min_sum / divide);
		}
		else
			return ((float)Decv[n].realvalue / divide);
	}
	if (HOUR == date){
		if (Decv[n].hour_count){
			if (class == AVER)
				return ((float)Decv[n].hour_sum / Decv[n].hour_count / divide);
			else if (class == MIN)
				return ((float)Decv[n].hour_min / divide);
			else if (class == MAX)
				return ((float)Decv[n].hour_max / divide);
			else if (class == SUM)
				return ((float)Decv[n].hour_sum / divide);
		}
		else
			return ((float)Decv[n].realvalue / divide);
	}
	if (DAY == date){
		if (Decv[n].day_count){
			if (class == AVER)
				return ((float)Decv[n].day_sum / Decv[n].day_count / divide);
			else if (class == MIN)
				return ((float)Decv[n].day_min / divide);
			else if (class == MAX)
				return ((float)Decv[n].day_max / divide);
			else if (class == SUM)
				return ((float)Decv[n].day_sum / divide);
		}
		else
			return ((float)Decv[n].realvalue / divide);
	}
}

/* 将传入数据组成一个十六进制数并返回 */
int getValue(unsigned char *data, int len)
{
	int result;
	int i;
	unsigned char *p;
	p = data;
	result = *p;
	/* 每次向左移动8bit，最终组成一个十六进制数 */
	for (i = 1; i < len; i++)
	{
		result = (result << 8);
		p = p + 1;
		result |= (*p); //按位或后赋值
	}
	return result; //返回被拼接的十六进制数
}

void setValue(unsigned char *des, int result, int len)
{
	unsigned char *p;
	p = des;
	if (len == 4){
		p[0] = ((result >> 24) & 0x00ff);
		p = p + 1;
		len = 3;
	}
	if (len == 3){
		*p = ((result >> 16) & 0x00ff);
		p = p + 1;
		len = 2;
	}
	if (len == 2){
		*p = ((result >> 8) & 0x00ff);
		p = p + 1;
		*p = (result & 0x00ff);
	}
}

void addAverage(int n, int value, averagetT *str, float divide)
{
	averagetT *Decv = str;
	float value1 = 0.0;
	if (divide < 0.00001)
		divide = 1.0;
	if (str == averRecArr){
		ParaT testP = para[n];
		setValue(&buffer_result[testP.desOffset], value, testP.len);
	}
	n--;

	Decv[n].realvalue = value;
	Decv[n].fl_realvalue = value / divide;
	if (0 == Decv[n].real_count){
		Decv[n].real_min = value;
		Decv[n].real_max = value;
	}
	else{
		if (value < Decv[n].real_min)
			Decv[n].real_min = value;
		if (value > Decv[n].real_max)
			Decv[n].real_max = value;
	}
	Decv[n].real_sum += value;
	Decv[n].real_count += 1;

	if (0 == Decv[n].min_count){
		Decv[n].min_min = value;
		Decv[n].min_max = value;
	}
	else{
		if (value < Decv[n].min_min)
			Decv[n].min_min = value;
		if (value > Decv[n].min_max)
			Decv[n].min_max = value;
	}
	Decv[n].min_sum += value;
	Decv[n].min_count += 1;

	if (0 == Decv[n].hour_count){
		Decv[n].hour_min = value;
		Decv[n].hour_max = value;
	}
	else{
		if (value < Decv[n].hour_min)
			Decv[n].hour_min = value;
		if (value > Decv[n].hour_max)
			Decv[n].hour_max = value;
	}
	Decv[n].hour_sum += value;
	Decv[n].hour_count += 1;

	if (0 == Decv[n].day_count){
		Decv[n].day_min = value;
		Decv[n].day_max = value;
	}
	else{
		if (value < Decv[n].day_min)
			Decv[n].day_min = value;
		if (value > Decv[n].day_max)
			Decv[n].day_max = value;
	}
	Decv[n].day_sum += value;
	Decv[n].day_count += 1;
}

void GetSensorValue(int i,int port_ith)
{
	int result = 0;
	ParaT testP;
	testP = para[i];
	tcflush(fdArray[port_ith], TCIFLUSH);
	result = readData(fdArray[port_ith], buffer, testP.lenRes, testP.cmd, testP.lenCmd, testP.cmd, 1, READ_BY_LEN);
	if (result == FALSE)
		result = readData(fdArray[port_ith], buffer, testP.lenRes, testP.cmd, testP.lenCmd, testP.cmd, 1, READ_BY_LEN);
	if(result != FALSE){
		result = getValue(&buffer[testP.lenRes - 2], LEN_MODBUS_CRC);
		result = check_modbus_crc(buffer, (testP.lenRes - 2), result);
		if(result != FALSE){
			result = getValue(&buffer[testP.srcOffset], testP.lenSrc);
			setValue(&buffer_result[testP.desOffset], result, testP.len);
			addAverage(i, result, averRecArr, 1.0);
		}
	}
}

void GetSmokeValue()
{
	GetSensorValue(SMOKE,SERIAL_485_2);
}

void GetWindSpeedValue()
{
	GetSensorValue(WIND_SPEED, SERIAL_485_2);
}

void GetWindDirectionValue()
{
	GetSensorValue(WIND_DIRECTION, SERIAL_485_2);
}

void GetTemperatureValue()
{
	GetSensorValue(TEMPERATURE, SERIAL_485_2);
}

void GetHumidtyValue()
{
	GetSensorValue(HUMIDITY, SERIAL_485_2);
}

void GetRainValue()
{
	GetSensorValue(RAINALL, SERIAL_485_2);
}

/*获取传感器数据控制函数*/
int getResult(int n)
{
	int i, j;
	int temp;
	i = n;
	j = i + 1;
	int num_sensor = getSenserNum();
	if (j > num_sensor)
		j = num_sensor;

	for (i = n; i < j; i++)
	{
		func[i]();
	}
	printStyle("\nmax j = %d\n", j);
	if (i >= num_sensor)
		i = 0;
	return i;
}