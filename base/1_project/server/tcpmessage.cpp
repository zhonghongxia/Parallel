//tcpmessage.cpp
#include "tcpmessage.h"

using std :: cout;
using std :: endl;

bool tcp_message::decode_header()//将报头的4字节字符串转换成数字
{
	body_len_ = *(int *)data_;
	if (body_len_ > max_body_len)
	{
	  body_len_ = 0;
	  return false;
	}
	return true;
}
void tcp_message::encode_header()//把数据部分大小编码成字符串
{
	memcpy(data_, &body_len_, header_len);
}
void tcp_message::decode_to_my_message(struct my_message &msg)	
{
	gettimeofday(&msg.start_time,NULL); //gettimeofday(&start,&tz);
	msg.keep_on = *(int *)(data_+header_len);
	msg.file_content_len = *(int *)(data_+header_len+KEEPON_LEN);
	msg.source_id = *(int *)(data_+header_len+KEEPON_LEN+FILECONTENTLEN_LEN);
	sprintf(msg.file_name ,"%s" ,data_+header_len+KEEPON_LEN+FILECONTENTLEN_LEN + SOURCEID_LEN);
	msg.file_content = data_+header_len+KEEPON_LEN+FILECONTENTLEN_LEN + SOURCEID_LEN+FILENAME_LEN;
}
void tcp_message::encode_my_message_result(struct my_message &msg )
{		
	clear();
	cout<<"remeber to set result_num"<<endl;
	gettimeofday(&msg.end_time,NULL); 
	set_body_len(msg.result_len + 20);//返回的是文件名加上url
	encode_header();	
	sprintf(data_+ header_len , "%s",msg.file_name);
	memcpy(data_+FILENAME_LEN + header_len, msg.result,msg.result_len);
}
