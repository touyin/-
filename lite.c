#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

char exist_sql();
void table_Init();
void get_time();
int date_exist();
void add_ljt();

char buf[1024];
char sql_buf[100];
sqlite3* sql_fd;

int exist_callback(void *arg, int column_size, char *column_value[], char *column_name[])
{
	int* count=(int*)arg;
	*count = atoi(column_value[0]);
	return 0;
}

int select_callback(void *arg, int column_size, char *column_value[], char *column_name[])
{	
	int count = atoi(column_value[2]) + 1;
	memset(sql_buf,'\0',strlen(sql_buf));

	sprintf(sql_buf,"update ljt set %s=%d where id=%s;",(char*)arg,count,column_value[0]);
	
	
	sqlite3_exec(sql_fd,sql_buf,NULL,NULL,NULL);
	
	return 0;
}
void sql_main(char* source)
{
	
	int date_flag=-1;
	char exist_sql_flag = exist_sql();
	sqlite3_open("sql.db",&sql_fd);

	if(exist_sql_flag == 0){
		table_Init();
	}
	date_flag =  date_exist();
	//辨别当天日期是否存入
	if(date_flag == -1){
		printf("date_exist() error\n");
		exit(-1);
	}else if(date_flag == 0){
		//如果没有数据，那么添加一条
		add_ljt();
	}
	printf("datetime=%s,strlen()=%ld\n",buf,strlen(buf));

	//更新对应数据
	memset(sql_buf,'\0',strlen(sql_buf));
	sprintf(sql_buf,"select * from ljt where dateTime=\"%s\";",buf);
	sqlite3_exec(sql_fd,sql_buf,select_callback,(void*)source,NULL);

	sqlite3_close(sql_fd);
}

void add_ljt()
{
	memset(sql_buf,'\0',strlen(sql_buf));
	sprintf(sql_buf,"insert into ljt values(NULL,\"%s\",0,0,0,0,0,0);",buf);
	sqlite3_exec(sql_fd,sql_buf,NULL,NULL,NULL);
}

int date_exist()
{
	int count=-1;
	get_time();
	memset(sql_buf,'\0',strlen(sql_buf));
	snprintf(sql_buf,sizeof(sql_buf),"select count(*) from ljt where dateTime=\"%s\";",buf);
	
	sqlite3_exec(sql_fd,sql_buf,exist_callback,&count,NULL);
	//printf("count=%d\n",count);
	return count;
}


void get_time()
{
	//获取时间
	memset(buf,'\0',strlen(buf));
	time_t now;
	struct tm* tm_now;
	time(&now);
	tm_now = localtime(&now);
	strftime(buf,200,"%Y-%m-%d",tm_now);
}

void table_Init(){
	char* table="create table ljt("\
				 "id INTEGER PRIMARY KEY AUTOINCREMENT,"\
				 "dateTime char(50)	NOT NULL,"\
				 "hc_open INT DEFAULT 0,"\
				 "hc_close INT DEFAULT 0,"\
				 "voice_open INT DEFAULT 0,"\
				 "voice_close INT DEFAULT 0,"\
				 "socket_open INT DEFAULT 0,"
				 "socket_close INT DEFAULT 0"\
				 ")";
	sqlite3_exec(sql_fd,table,NULL,NULL,NULL);
}

//检测是否存在sql.db文件
char exist_sql()
{	
	memset(buf,'\0',strlen(buf));
	FILE* file = popen("ls","r");

	fread(buf,1024,1,file);

	if(strstr(buf,"sql.db") == NULL){
		return 0;
	}
	return 1;
}	
