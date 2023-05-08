#include <stdio.h>
#include <wiringPi.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "file.h"
#include "lite.h"

#define echo 9
#define trig 10
#define echo_value digitalRead(echo)

extern double dis;
extern char flag;
extern int voice_cmd;
extern char s_flag;
extern int file_fd;

void switch_file(int fd,int log);

void hc_Init()
{
	//设置echo为输出模式
	pinMode(echo,INPUT);
	//设置trig为输入模式
	pinMode(trig,OUTPUT);
}

double getDistance()
{
	double dis;
	struct timeval time_start;
	struct timeval time_stop;

	//怎么让它发波
	//Trig，给Trig端口至少10us的高电平
	digitalWrite(echo,LOW);
	digitalWrite(trig,LOW);
	usleep(5);
	digitalWrite(trig,HIGH);
	usleep(10);
	digitalWrite(trig,LOW);

	//怎么知道开始发了
	//Echo信号，由低电平跳转到高电平，表示开始发送波
	while(!echo_value);
	gettimeofday(&time_start,NULL);

	//怎么知道接收了返回波
	//Echo，由高电平跳转回低电平，表示波回来了

	while(echo_value){};
	gettimeofday(&time_stop,NULL);

	//怎么算时间
	//Echo引脚维持高电平的时间！
	long time = time_stop.tv_usec-time_start.tv_usec + (time_stop.tv_sec-time_start.tv_sec)*1000000;

	dis = (340/2)*time*0.0001;

	return dis;
}


void* hc_pthread_handler(void* arg)
{	
	char pre_voice=0;
	char pre_s=0;

	while(1){
		dis=getDistance();
		
		//控制决定是否开关盖，并且写入也在这里写入
		if(dis<10 || voice_cmd == 1 || s_flag==1){
			pre_voice = voice_cmd;
			pre_s = s_flag;

			//只响一次
			if(flag == 0){
				flag=1;
				strcpy((char*)arg,"open");

				switch_file(file_fd,1);
				sleep(2);
			}
		}else if(flag == 1){
			
			//语音关闭和socket关闭是看pre*变量
			
			if(pre_voice==1 && voice_cmd==0){
				switch_file(file_fd,4);
			}else if(pre_s==1 && s_flag==0){
				switch_file(file_fd,3);
			}else if(dis>10 && voice_cmd == 0 || s_flag==0){
				switch_file(file_fd,5);
			   	//距离关闭是只有当三者全为0时，才关闭
			}
			strcpy((char*)arg,"close");
			flag=0;
		}
		
		printf("current cmd:%s\n",(char*)arg);
		sleep(1);
	}

	pthread_exit(NULL);
}

void switch_file(int fd,int log)
{
	if(log==1){
		if(s_flag==1){
			file_main(fd,"socket_open");
			sql_main("socket_open");
		}else if(voice_cmd==1){
			file_main(fd,"voice_open");
			sql_main("voice_open");
		}else {
			file_main(fd,"hc_open");
			sql_main("hc_open");
		}
	}else if(log == 4){
		file_main(fd,"voice_close");
		sql_main("voice_close");
	}else if(log == 3){
		file_main(fd,"socket_close");
		sql_main("socket_close");
	}else if(log==5){
		file_main(fd,"hc_close");
		sql_main("hc_close");
	}


}


