#include "network.h"
#include "data.h"

int ctrl_time_sec_up; //上传数据秒控制位
time_t time_now = 0;
int systime_sec = 0; //更改时间精确定位到秒
int systime_sec_diff = 0;
time_t time_before = 0;
unsigned char buffer_temp[2500] = {0};
int sockFdArray[NUM_TOTAL_IP]; //套接字，连接应用层和传输之间的桥梁。

/* 单个客户端指定TCP连接 包含：socket()和connect() */
int tcpClientConnetSingle(int n, int isBlock)
{
    int fd;
    struct sockaddr_in remote_addr; // Host address information
    char ip[16];
    if(sockFdArray[n] == INVALID_FD)
    {// 如果当前套接字数组为-1，可用
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        { //socket(IPv4，TCP传输，默认协议) 来打开一个网络通讯端口，并返回其文件描述符
            return (0);
        }
        sockFdArray[n] = fd; //然后更新当前文件描述符，即socket数组，sockFdArray[ n ]里存的就是本次打开的串口
    }
    else
        fd = sockFdArray[n];

    ipToString(ip, n); // 获取并转换从config文件初始化的ip地址
    /* Fill the socket address struct */
    remote_addr.sin_family = AF_INET; // Protocol Family 地址结构类型 AF_INET=IPv4
    remote_addr.sin_port = htons(getDestPort(n)); // Port number 端口号

    printStrs("connet sigle ip:%s\n", ip);
    printStyle("port == %04d\n", getDestPort(n));
    inet_pton(AF_INET, ip, &remote_addr.sin_addr); // 转换ip字符串地址，由sin_addr存放二进制结果地址。
    if (isBlock == FALSE)
        setFdNoBlock(fd);

    if (connect(fd, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr)) == -1)
    { // 用于建立和指定socket的连接
        return FALSE;
    }
    else{
        return fd;
    }
}

int tcpConnet(int n)
{
    if (sockFdArray[n] == INVALID_FD){
        tcpClientConnetSingle(n, FALSE);
    }
    else{
        printf("ERROR: Obtain new Socket Despcritor error.\n");
        close(sockFdArray[n]);
        sockFdArray[n] = INVALID_FD;
    }
    return TRUE;
}

void closeConnet(int n)
{
    if (sockFdArray[n] != INVALID_FD){
        close(sockFdArray[n]);
        sockFdArray[n] = INVALID_FD;
    }
}

int virifyResponse(int fd, const unsigned char *buf, int len, int nt)
{
    int i=0;
    int j=0;
    int temp;
    int result;
    int tempFdArray[1];
    unsigned char des;
    result = TRUE;
    tempFdArray[0] = fd;
    temp = checkFdStatus(tempFdArray, 1, TRUE, nt);
    if (temp != FALSE) {
        while (i < len){
            if (FALSE == read(fd, &des, 1)){
                result = FALSE;
                break;
            }
            if (des == buf[i])
                i += 1;
            else{
                j = j + 1;
                if ((i > 0) || (j > len)){
                    result = FALSE;
                    break;
                }
            }
        }
    }
    else
        result = FALSE;
    return result;
}

int tcpVerifyResponse(int n, unsigned char *resEpx, int len)
{
    return virifyResponse(sockFdArray[n], resEpx, len, 3);
}

int tcpSend(int n, unsigned char *src, int len, const unsigned char *resEpx, int resLen)
{
    int i;
    i = 0;
    int num;
    int result;
    num = -1;
    result = TRUE;
TCPSEND_AGAIN:
    if ((sockFdArray[n] != INVALID_FD) && (FALSE != checkFdStatus(&sockFdArray[n], 1, FALSE, 1))){
        if ((num = send(sockFdArray[1], src, len, 0)) == -1) {
            closeConnet(n);                              //先断开之前的连接
            tcpConnet(n);                                //再建立当前要通信的连接
            usleep(500);
            result = send(sockFdArray[n], src, len, 0); //向服务器发送数据
            if (result == -1)
                printf("errno:%d\n", errno);
        }
        printf("result = %d\n", result);

        if(resEpx != NULL && result != FALSE) {
            usleep(50000);
            result = tcpVerifyResponse(n, resEpx, resLen);
            if (result == FALSE && i < 3){
                i++;
                goto TCPSEND_AGAIN;
            }
        }
    }
    else{
        closeConnet(n);
        tcpConnet(n);
        printStr("nsockfd not available, do not send, close tcp connetion ");
        result = FALSE;
    }
    return result;
}

char *getPointDataBuffer_temp( )
{
    int len = 0;
    return &buffer_temp[len];
}

int getTimeStr(unsigned char *des, int CN)
{
    int len = 0;
    time_t rawtime;
    struct tm *timeinfo;
    extern int systime_sec_diff;

    time(&rawtime);
    if(CN == 2011){
        rawtime -= systime_sec_diff;
        timeinfo = localtime(&rawtime);
        printStrs("%s", asctime(timeinfo));
        if (ctrl_time_sec_up == 1){
            len = sprintf(des, "%4d%02d%02d%02d%02d%02d", 1900 + timeinfo->tm_year, (timeinfo->tm_mon + 1),
                          timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        }
        else{
            len = sprintf(des, "%4d%02d%02d%02d%02d%02d", 1900 + timeinfo->tm_year, (timeinfo->tm_mon + 1),
                          timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, 0);
        }
    }
    else{
        timeinfo = localtime(&rawtime);
        len = sprintf(des, "%4d%02d%02d%02d%02d%02d", 1900 + timeinfo->tm_year, (timeinfo->tm_mon + 1),
                      timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, 0);
    }
    return len;
}

int getCRC212(unsigned char *puchMsg, unsigned int usDataLen)
{
    unsigned int i, j, crc_reg, check;
    crc_reg = 0xFFFF;
    for (i = 0; i < usDataLen; i++){
        crc_reg = (crc_reg >> 8) ^ puchMsg[i];
        for (j = 0; j < 8; j++){
            check = crc_reg & 0x0001;
            crc_reg >>= 1;
            if (check == 0x0001)
                crc_reg ^= 0xA001;
        }
    }
    return crc_reg;
}

int getData_HJ212(int CN, unsigned char *id, int len_id)
{
    unsigned char *temp;
    int offset = 0;
    char passwd[7] = "123456";
    offset += sprintf(&temp[offset], "##0000");
    offset += sprintf(&temp[offset], "QN=");
    offset += getTimeStr(&temp[offset], CN);
    offset += sprintf(&temp[offset], "%03d;", t.millitm);
    offset += sprintf(&temp[offset], "ST=22;CN=%d;", CN);
    offset += sprintf(&temp[offset], "PW=%s", passwd);
    offset += sprintf(&temp[offset], ";Flag=5");
    offset += sprintf(&temp[offset], ";CP=&&DataTime=");
    offset += getTimeStr(&temp[offset], CN);
    temp[offset++] = ';';
    int size = 6;
    int i = 0;
    int timeFlag;
    float min_value;
    float aver_value;
    float max_value;
    float sum_value;
    /*!循环获取传感器数据并封装成数据包!*/
    for(i = 1; i <= size; i++){
        int value = getSenserValue(i);
        int precision = 2;
        if(CN == 2011){
            offset += sprintf(&temp[offset], "%s-Rtd=%0.*f,%s-Flag=%c;",
                              GetParaName(i), precision, value, GetParaLabel(i), 'N'); /*数据包封装(字符拼接)*/
        }
        else if (CN == 2021 || CN == 2031){
            switch (CN){
                case 2021:
                    timeFlag = MINUTE;break;
                case 2031:
                    timeFlag = HOUR;break;
            }
            min_value = getDataAverMinMaxSum(i, timeFlag, MIN);
            aver_value = getDataAverMinMaxSum(i, timeFlag, AVER);
            max_value = getDataAverMinMaxSum(i, timeFlag, MAX);
            sum_value = getDataAverMinMaxSum(i, timeFlag, SUM);
            /*其他时段212数据包封装*/
            offset = HJ212_Sprint(temp, offset, GetParaName(i), min_value, aver_value,
                                  max_value, sum_value, precision, "N");
        }
    }

    i = getCRC212(&temp[6], offset - 6);
    offset += sprintf(&temp[offset], "%04X", i); //封装212CRC校验码
    temp[offset++] = '\0';
    offset -= 1;
    temp[offset] = '\0';
    return offset;
}

 void sendService(int CN)
{
    int len = 0;
    hwclockTimeSynToSystem();                        //硬件同步到软件时间
    len = getData_HJ212(CN, ID, strlen(ID));         //数据包组装
    writeFile(getPointDataBuffer_temp(),len, CN);    //写入内存卡

    len = getData_HJ212(CN, ID, strlen(ID)); //数据包组装
    tcpSend(1,getPointDataBuffer_temp(), len, NULL, 0);
    checkNet(CN, len);
    if (CN == 2011){
        time(&time_real_Ago);
    }
}

void uploadServer(struct tm *timeinfo)
{
    int time_mul = 1;
    int upTime = 60;
    static char first = 0;
    static int countMinTime;
    static int countHourTime;
    static int countDayTime;
    if (!first){ // just once
        countMinTime = timeinfo->tm_min;
        countHourTime = timeinfo->tm_hour;
        countDayTime = timeinfo->tm_mday;
        first = 1;
    }
    upTime = getUpTime();
    if(upTime != 60){
        ctrl_time_sec_up = 1; //秒不固定为0
        time(&time_now);
        systime_sec = time_now - time_before;        //距离上一次上传的时间
        if (systime_sec >= upTime) {                 //差值大于上传间隔
            time_mul = systime_sec / upTime;         //倍数,一般都是一倍
            systime_sec_diff = systime_sec % upTime; //延迟量(在上传的地方减去)
            printf("*********into ctroltime ready combine data\n");
            printf("***********before ctrotime timebefore1 is %ld, tm_gz is %ld\n", time_before, time(0));
            // printf("***********before ctrotime timebefore1 is %ld, tm_gz is %ld\n",time_before1,tm_gz);
            sendService(2011);
            time_before += (getUpTime() * time_mul); //如果经过上传时间的N倍都没上传数据，这期间的数据就舍弃
            systime_sec_diff = 0;                     //恢复
            ClearData(0);
            printf("*******out ctroltime and go on\n");
        }
        ctrl_time_sec_up = 0;
    }

    if (timeinfo->tm_hour != countHourTime){
        countHourTime = timeinfo->tm_hour;
        countMinTime = timeinfo->tm_min;
        sendService(2021);
        ClearData(1);
    }

    if (timeinfo->tm_mday != countDayTime)
    {
        countDayTime = timeinfo->tm_mday;
        countHourTime = timeinfo->tm_hour;
        countMinTime = timeinfo->tm_min;
        sendService(2031);
        ClearData(2);
        commandExecute("rdate -s 132.163.96.4 && hwclock -w", "w"); //同步时间
    }
}
