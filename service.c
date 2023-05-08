#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

extern char s_flag;
void switch_write(char* buf_r);
void error_handler(char* reason)
{
	printf("%s error\n",reason);
	perror("why");
	exit(-1);

}

int socket_connect=0;

void* write_pthread_handler(void* arg)
{
	char write_buf[124];
	while(1){
		memset(write_buf,'\0',strlen(write_buf));
		strcpy(write_buf,(char*)arg);

		write(socket_connect,write_buf,strlen(write_buf));
		sleep(1);	
		fflush(stdout);
	}

	pthread_exit(arg);
}

//启动socket
void socket_main(int share_id,char* argv)
{
	
	
	int socket_fd=0;
	//1.创建套接字
	//int socket(int domain, int type, int protocol)
	if((socket_fd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		error_handler("socket()");
	}
	printf("socket success\n");

	//2.为套接字添加信息：IP地址和端口
	//int bind(int sockfd,const struct sockaddr *addr,socklen_t addrlen)
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv));
	if(inet_aton("192.168.31.75",&(addr.sin_addr)) == 0){
		error_handler("inet_aton()");
	}

	if(bind(socket_fd,(struct sockaddr*)&addr,sizeof(addr)) == -1){
		error_handler("bind()");
	}
	printf("bind success\n");
	
	//3.监听网络连接
	//int listen(int sockfd,int backlog);
	if(listen(socket_fd,2) == -1){
		error_handler("listen()");
	}
	printf("listen success\n");

	//4.监听到有客户端接入，接收一个连接
	
	//int accept(int sockfd,struct sockaddr* addr,socklen_t* addrlen)
	int addr_len = sizeof(addr);
	if((socket_connect = accept(socket_fd,(struct sockaddr*)&addr,&addr_len))==-1){
		error_handler("accept()");
	}
	//5.数据交互
	void* arg = shmat(share_id,0,0);//apply share appoint
	//read()、write()
	char buf[1024] = "hello,this is service\n";

	//开启一个子线程负责给客户端一直发送当前垃圾桶的状态
	//主线程负责发送	
	pthread_t socket_pthread_tidp;
	if(pthread_create(&socket_pthread_tidp,NULL,write_pthread_handler,arg)){
		error_handler("pthread_create()");
	}
	
	write(socket_connect,buf,strlen(buf));

	while(1){
		memset(buf,'\0',strlen(buf));
		
		if(read(socket_connect,buf,1024) == -1){
			error_handler("read()");
			break;
		}	
		switch_write(buf);
		printf("client:%s\n",buf);
		fflush(stdout);
		sleep(1);
	}
	
	//收取线程资源
	pthread_join(socket_pthread_tidp,NULL);

	//6.关闭套接字，断开连接
	//close()
	close(socket_fd);
}

//通过读取到的数据发送相应信号
void switch_write(char* buf_r)
{
	if(strstr(buf_r,"open")){
		//开盖
		kill(getppid(),SIGUSR1);
	}else if(strstr(buf_r,"close")){
		//关盖
		kill(getppid(),SIGUSR2);
	}else {
		//发送指令错误消息
		write(socket_connect,"cmd error!!!",sizeof("cmd error!!!"));
	}
}


