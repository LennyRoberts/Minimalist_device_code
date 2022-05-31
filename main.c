#include "global.h"
#include "data.h"

int main(void)
{
	int serial_open_status = 0;
	int test_123=0;
	int num = 0;
	int temp = 0;
	int upTime;
	int i_num = 0;
	int weather_num = 0;
	
	struct tm* timeinfo; 
	int countMinTime_rain;
	int countSecTime_evaportion = 60;                                             //86400
	char count_wea=5;                                                             //进入循环要显示天气预报，要是GPS定位读不到多读几次
	int time_mul = 1;
	time_t SecTime = 0L;
	setenv("MALLOC_TRACE", "trace.log",1);
	mtrace();
	commandExecute("mount -o rw,remount /media/mmcblk0p1", "w");                  //防止内存卡变成只读
	commandExecute("wr sh -c \"echo hosts:files dns>/etc/nsswitch.conf\"", "w");  //写入DNS
	setenv("TZ", "GMT-8", 1);
	commandExecute("hwclock -s ", "w");                                           //同步硬件时间到系统时间
	
	initGPIO();                       //初始化gpio口
	initVariable();                   //初始化变量
    initSystem();                     //传感器参数
	initActionStatus();               //初始化联动状态
	initResult();                     //写入参数默认值
	getID();                          //获取3个中心的id号

	serial_open_status = openAllSensers();                                       //串口+设备波特率
	if(serial_open_status){                                                      //串口无法正常打开
		commandExecute("wr sh -c \"echo 'open serial port errno!!!' >> osen_run.log\"", "w");
		exit(1);
	}


	print_config();


	srand(time(0)); 

	#if(!NOBEEP)
	startUpBeep();                                                             //蜂鸣器
	#endif
	printStyle("before send version uptime= %04X\n", getUpTime());
	ledSendVersion();                                                          //发送版本号
	// commandExecute("rdate -s 132.163.96.4 && hwclock -w", "w");                //同步时间
	commandExecute("rdate -s time.nist.gov && hwclock -w", "w");               //同步时间

	openWatchDog();                                                            //打开看门狗
	getAllSenserValue();                                                    //采集传感器数据入口函数

	timeinfo = localtime (&timeVar); 
	systime_min = timeinfo->tm_min;
	systime_sec = timeinfo->tm_sec;
	sendService(RECORD_SERVICE,2011);                                       //上传实时数据
	time_before1=time(NULL);


	printStr("\n\n-------------------------------------------------------\n");
	printStr("--------Init Finish and welcome use the device---------\n");
	printStr("-------------------------------------------------------\n\n");
	
	while(1){ 
		if(count_back != count){                                                         //减少CPU占用
			time(&time_real_Now);
			printf("\n----------------[upTimeDiff:%ld]--------------------\n", 
			       time_real_Now - time_real_Ago);                                       //距离上一次实时数据上传时间间隔

			count_back = count;
			tm_gz = time(NULL);
			timeinfo = localtime(&tm_gz);                                                //用于数据上传时机的控制

			getFristLineParam();                                                       //config第一行实时读取
	

			uploadServer(timeinfo);                                                   //上传到服务器
			temp = manageDangerAction( );                                            //联动管理

			num = getResult(num);                                                   //读传感器
			if(count >= getUpTime())
				count = count % getUpTime();

			feedWatchDog();                                                         //喂狗
			checkCommandNew();                                                      //上位机
		}else                                                                      //if count_back != count
			checkSockFdArray();                                                    //链路断开重连
		if(count_back == count)
			usleep(20);
	}

	closeWatchDog();
	return 1;
}