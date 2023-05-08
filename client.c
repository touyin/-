#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>


int socket_id =0;
void error_handler(char* reason){

	printf("%s error\n",reason);
	perror("why?");
	exit(-1);
}

void* chail_read_pthread_handler(void* arg)
{	
	char read_buf[1024];

	while(1){
		memset(read_buf,'\0',strlen(read_buf));
		read(socket_id,read_buf,1024);
		printf("service:%s\n",read_buf);
		fflush(stdout);
	}
	
	pthread_exit(arg);
}
//子线程负责读，主线程负责写
int main(int argc,char** argv)
{
	if(argc<2){
		error_handler("argc<2");
	}
	//int socket(int domain, int type, int protocol)
	if((socket_id = socket(AF_INET,SOCK_STREAM,0)) == -1){
		error_handler("socket()");
	}
	printf("socket success\n");

	//int connect(int sock, struct sockaddr *serv_addr, socklen_t addrlen);
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));

	addr.sin_family=AF_INET;
	addr.sin_port=htons(atoi(argv[1]));
	inet_aton("192.168.31.75",&(addr.sin_addr));

	if(connect(socket_id,(struct sockaddr*)&addr,sizeof(addr)) == -1){
		error_handler("connect()");
	}

	//write()、read()
	//create child process
	pthread_t read_tidp;
	int arg=11;
	if(pthread_create(&read_tidp,NULL,chail_read_pthread_handler,(void*)&arg));
	
	char write_buf[1024];
	while(1){
		memset(write_buf,'\0',1024);
		scanf("%s",write_buf);
		write(socket_id,write_buf,strlen(write_buf));
		fflush(stdin);
	}
	pthread_join(read_tidp,NULL);
	//close()
	close(socket_id);
	return 0;
}
