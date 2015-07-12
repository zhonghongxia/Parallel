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
}__attribute__( ( packed ) );




#endif
