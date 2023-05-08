#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <wiringPi.h>
#include <errno.h>

#define BEEP 6
#define beep_high digitalWrite(BEEP,HIGH)
#define beep_low digitalWrite(BEEP,LOW)

extern char flag;

void beep_Init()
{
	pinMode(BEEP,OUTPUT);
	digitalWrite(BEEP,HIGH);
}

pthread_mutex_t mutex;
pthread_cond_t cond;
//定时d秒，j秒后解锁
//int d,j;
//定时器的初始化
//指定定时几秒
struct timespec timeD_init()
{
	struct timespec timeD;
	timespec_get(&timeD,TIME_UTC);
	timeD.tv_sec += 10;

	return timeD;
}

void* beep_wait_handler(void* arg)
{
	//等待主线程释放mutex锁
	pthread_mutex_lock(&mutex);

	pthread_mutex_unlock(&mutex);
	//开始sleep
	//等待flag变为0
	while(flag != 0);

	pthread_cond_signal(&cond);
	pthread_exit(arg);
}

//beep main pthread
//beep函数的主线程
void* beep_pthread_handler(void* arg)
{
	struct timespec timeD;

	//锁初始化
	pthread_mutex_init(&mutex,NULL);
	pthread_cond_init(&cond,NULL);

	//开启线程
	pthread_t beep_tidp;
	char* ret=NULL;

	while(1){

		beep_high;
		while(flag==0);
		beep_low;
		sleep(1);
		beep_high;

		pthread_mutex_unlock(&mutex);
		pthread_mutex_lock(&mutex);

		printf("lock run\n");
		if(pthread_create(&beep_tidp,NULL,beep_wait_handler,arg) != 0){
			printf("beep_pthread create error\n");
			perror("why");
			exit(-1);
		}
		timeD=timeD_init();

		//主线程初始化完毕，释放锁，让子线程开始计时
		pthread_mutex_unlock(&mutex);
		//等待子线程计时完毕，或者超时
		while(pthread_cond_timedwait(&cond,&mutex,&timeD) == ETIMEDOUT){
			printf("flag=%d\n",flag);
			beep_low;
			sleep(2);
			break;
		}
		pthread_mutex_unlock(&mutex);
		pthread_join(beep_tidp,(void**)&ret);
	}

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	return 0;
}
