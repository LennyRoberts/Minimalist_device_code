# 一种基于LoRa组网的森林火灾监测系统代码实现
- 该程序使用交叉编译器编译，使用在armv7的核心开发板中(RAM:256MB, ROM256MB)
- 交叉编译器:
	名称：gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux
	下载地址：https://releases.linaro.org/archive/14.09/components/toolchain/binaries/
	请访问Linaro的官方服务器，下载所示的交叉编译链工具，并按所示教程正确安装
	
-交叉编译器安装参考:
	1."arm-linux-gnueabihf-" 编译程序时因缺少"ib32stdc++6"库而使交叉编译器未被发现
	解决：安装对应的库即可=> sudo apt-get install lib32stdc++6
	2.将文件解压到自定义安装目录：我使用的是：/home/user/ctools 或者 /opt/ctools
	3.在/etc/profile中添加全局环境变量：
		export PATH=/home/user/ctools/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin:$PATH
	  或者：export PATH=/opt/ctools/gcc-linaro-arm-linux-gnueabihf-4.9-2014.09_linux/bin:$PATH
	4.执行: ". /etc/profile"
	

| 传感器 | 描述 |
| :---: | :--- |
| 烟雾 | --- |
| 雨量 | --- |
| 温度 | --- |
| 湿度 | --- |
| 风速 | --- |
| 风向 | --- |

