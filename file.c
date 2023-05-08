#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void error_handler(char* reason);
void file_Init(int fd);
void config_ration(int fd);
time_t now;
//today date
char now_date[50];

//file data
char buf[500];
char addr_buf[200];
time_t now;
struct tm* tm_now;
char datetime[200];
int gd_len=0;

//数字转换
char num_buf[10];
//得到数字
int getNum(char* addr)
{
	char* p = addr;
	
	memset(num_buf,'\0',strlen(num_buf));
	int i=0;
	while((*p) != '\n'){
		num_buf[i++] = *p;
		p++;
	}
	printf("num1=%s\n",num_buf);
	return atoi(num_buf);
	
}
//将数字塞进去
void setNum(char* addr,int int_num)
{
	char* p = addr;
	memset(num_buf,'\0',strlen(num_buf));
	
	printf("sprintf\n");
	sprintf(num_buf,"%d%c",int_num,'\0');
	printf("stop sprintf\n");
	int i=0;
	while(num_buf[i] != '\0'){
		*p=num_buf[i++];
		p++;
	}

}

//第二个参数是指，来源是什么，如果是hc_open,那么sorce=hc_open
void date_Init(int fd,char* sorce)
{	
	//注意光标位置，运行到这里时，光标要么在首部(<450)，要么在基于尾部的450处(>450)
	//每一次开关盖，都获取当天日期，然后比对，如果没有那么运行初始化
	//获取
	time(&now);
	tm_now = localtime(&now);
	strftime(datetime,200,"%Y-%m-%d\n",tm_now);
	char* str_addr=NULL;
	//比对
	if((str_addr = strstr(buf,datetime)) == NULL){
		//如果没有，那么就代表是新的一天，需要重新建立数据
		//运行完毕后buf中会重新填满数据
		config_ration(fd);
		str_addr = strstr(buf,datetime);
	}
	
	//运行到了这里，代表文件中是一定有对应日期的相应数据
	//str_addr一定是指向了第一次出现对应日期的首地址
	//移动光标到相应首地址位置
	int len = str_addr-buf;
	lseek(fd,len,SEEK_CUR);

	//那么重新获取相应的地址填充addr_buf
	//计算确认要读取多少个字节
	int str_num = strlen(buf)-len;
	
	//准备读取数据
	read(fd,addr_buf,str_num);
	//再次移动光标到addr_buf对应的首地址处
	lseek(fd,-str_num,SEEK_END);
	//获取对应数据的首地址
	char* num_addr = strstr(addr_buf,sorce);
	//移动num_addr到数据处
	num_addr = num_addr+strlen("hc_open     =");
	//将数字读取出来
	printf("addr_buf:%s\n",addr_buf);
	int num = getNum(num_addr);
	num = num+1;
	setNum(num_addr,num);

	write(fd,addr_buf,strlen(addr_buf));	
}

void file_main(int fd,char* sorce)
{
	
	gd_len = strlen("hc_open     =0   \n");
	file_Init(fd);

	date_Init(fd,sorce);
}

//读取文件，将光标移动到文件尾部，判断是否超过了450
//如果超过了，那么只读取450个字节，如果没有超过，那么全部读取出来
//这样做是为了尽量避免buf溢出
//这一个程序只负责移动光标以及填充buf
void file_Init(int fd)
{

	//移动光标到头部，保证进入这一段程序时，光标一定在头部
	lseek(fd,0,SEEK_SET);
	printf("file_main=%d\n",fd);		
	int file_num = lseek(fd,0,SEEK_END);
	lseek(fd,0,SEEK_SET);
	if(file_num>450){
		//大于450，那么移动光标
		lseek(fd,-450,SEEK_END);			
	}
	//冲刷buf
	memset(buf,'\0',strlen(buf));
	//读取文件
	read(fd,buf,450);
	//读取完毕，将光标移回去
	if(file_num>450){
		lseek(fd,-450,SEEK_END);
	}else {
		lseek(fd,0,SEEK_SET);
	}
}
void config_ration(int fd){
	
	//移动光标到尾部
	lseek(fd,0,SEEK_END);
	//写入数据
	//这里依然可以优化，也就是如果是不对齐的呢？
	//时间
	write(fd,datetime,strlen(datetime));
	//hc num
	write(fd,"hc_open     =0   \n",gd_len);
	write(fd,"hc_close    =0   \n",gd_len);
	//voice num
	write(fd,"voice_open  =0   \n",gd_len);
	write(fd,"voice_close =0   \n",gd_len);
	//socket num
	write(fd,"socket_open =0   \n",gd_len);
	write(fd,"socket_close=0   \n",gd_len);
	//写入完毕，再次重新刷新buf，(这是一个优化点)
	//printf("strlen=%ld\n",strlen("socket_open =0\n"));
	file_Init(fd);
}
