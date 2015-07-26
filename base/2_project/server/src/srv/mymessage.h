#ifndef MYMESSAGE_H
#define MYMESSAGE_H
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

struct my_message
{
	struct timeval start_time;
	struct timeval end_time;
	int keep_on;
	int file_content_len;
	int result_len;//返回的url的长度
	int source_id;
	char file_name[20];
	char *file_content;
	char *result;
	void print()
	{
		
		printf("这个函数打印的字符串并非是故事的所有 keepon : %d,file_content_len: %d,result_len : %d ,surce_id : %d,file_name:%s,file_content :%s , result: %s\n",keep_on,file_content_len,result_len,source_id,file_name,file_content,result);
	}
}__attribute__( ( packed ) );




#endif
