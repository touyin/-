#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include "uartTools.h"

int voic_fd;
extern int voice_cmd;

void voic_Init()
{
	if((voic_fd = myserialOpen("/dev/ttyS5",9600)) < 0){
		printf("mySerialOpen() error\n");
		perror("why");
		exit(-1);
	}
} 

void* voic_pthread_handler(void* arg)
{
	voic_Init();
	char cmd;
	printf("voic_fd=%d\n",voic_fd);

	while(1){
		cmd = serial_Get_char_one(voic_fd);
		
		printf("this is cmd=%c\n",cmd);
		switch(cmd){
			case 'O':
				printf("open\n");
				voice_cmd=1;
				break;
			case 'C':
				printf("close\n");
				voice_cmd=0;
				break;
		}
		while(serialDataAvail(voic_fd));	

		sleep(1);
	}
	pthread_exit(arg);

}
