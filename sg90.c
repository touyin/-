#include <stdio.h>
#include <wiringPi.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#define pwm 2

extern double dis;
extern char flag;

int jd=0;
int sg90_cnt=0;

void sg90_signal_handler(int signal)
{
	
    if(signal == SIGALRM){

        sg90_cnt++;
        if(sg90_cnt <= jd){
            digitalWrite(pwm,HIGH);
        }else {
            digitalWrite(pwm,LOW);
        }

        if(sg90_cnt == 40){
            sg90_cnt=0;
            digitalWrite(pwm,HIGH);
        }
    }
}


void timer_Init()
{
	//定时器
	struct itimerval timer;
  	//设定定时时长
    timer.it_interval.tv_sec=0;
    timer.it_interval.tv_usec=500;
    //多长时间后开始
    timer.it_value.tv_sec=1;
    timer.it_value.tv_usec=0;

    if(setitimer(ITIMER_REAL,&timer,NULL) == -1){
        fprintf(stderr,"%s","setitimer() error\n");
        exit(-1);
    }

    signal(SIGALRM,sg90_signal_handler);
}

void sg90_Init()
{
    pinMode(pwm,OUTPUT);
	timer_Init();
}

int sg90_process_handler(int share_id)
{
	char buf[124];
	//申请共享内存
	void* sg90_address = shmat(share_id,0,0);
    while(1){
        //子进程只起到一个控制舵机的功能
        //子进程不断从管道中获取值，一旦获取到相应的，那么就代表开关盖子
        memset(buf,'\0',24);
		//读
		strcpy(buf,(char*)sg90_address);
        if(strstr(buf,"open")){
            jd=5;
			
        }else if(strstr(buf,"close")){
            jd=0;
		
        }
        fflush(stdout);
    }
	shmdt(sg90_address);
    return 0;
}
