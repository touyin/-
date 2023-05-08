#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <signal.h>
#include "hc.h"
#include <string.h>
#include "sg90.h"
#include <pthread.h>
#include "beep.h"
#include <wiringSerial.h>
#include "voice.h"
#include "service.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

extern int jd;
extern int sg90_cnt;


double dis=0;
char flag=0;
char s_flag=0;

//舵机子进程
char buf[24];

//语音
int voice_cmd=0;
void handler(int signal);

int file_fd=0;

int main(int argc,char** argv)
{	

	if(argc<2){
		error_handler("argc<2");
	}

	//文件申请
	file_fd = open("record",O_RDWR|O_CREAT,0666);

	key_t key = ftok(".",12);
	int share_id = shmget(key,1024*4,IPC_CREAT|0600);
	void* arg = shmat(share_id,0,0);

	//初始化wiring为串口模式
	if(wiringPiSetup() == -1){
		error_handler("wiringPiSetup()");
	}

	//初始化hc
	hc_Init();
	printf("hcInit\n");
	//开启hc超声波线程
	pthread_t hc_tidp;
	if(pthread_create(&hc_tidp,NULL,hc_pthread_handler,arg) != 0){
		error_handler("hc_pthread_create() ");
	}

	//舵机相关
	int pid = fork();


	if(pid == 0){
		//初始化sg90舵机
		sg90_Init();
		sg90_process_handler(share_id);
		//第一个子进程退出
		return 0;
	}

	//初始化蜂鸣器
	beep_Init();
	//开启bb线程
	pthread_t beep_tidp;
	if(pthread_create(&beep_tidp,NULL,beep_pthread_handler,arg) != 0){
		printf("beep_tidp pthread_create error\n");
		perror("why");
		exit(-1);
	}

	//语音控制开关盖
	//voic_Init();
	pthread_t voic_tidp;
	if(pthread_create(&voic_tidp,NULL,voic_pthread_handler,arg) != 0 ){
		printf("voic_tidp pthread_create error\n");
		perror("why");
		exit(-1);
	}

	//注册信号
	signal(SIGUSR1,handler);
	signal(SIGUSR2,handler);
	//开启socket子进程
	int socket_id = fork();
	if(socket_id == 0){
		socket_main(share_id,argv[1]);
		return 0;
	}
		

	//回收资源
	pthread_join(hc_tidp,NULL);
	pthread_join(beep_tidp,NULL);
	pthread_join(voic_tidp,NULL);
	wait(NULL);
	close(file_fd);
	shmdt(arg);
	shmctl(share_id,IPC_RMID,0);

	return 0;
}

void handler(int signal)
{
	switch(signal){
		case SIGUSR1:
			//open
			s_flag=1;
			break;
		case SIGUSR2:
			//close
			s_flag=0;
			break;
	}
}

