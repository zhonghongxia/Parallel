//tcpmessage.hpp

/***********************************************
 _____________________________________________
|					|						  |
|___________________|_________________________|
<----header_len---->|<--------body_len------->

***********************************************/
#ifndef TCPMESSAGE_H
#define TCPMESSAGE_H
#include "protocol.h"
#include "mymessage.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/time.h>
using std ::cout;
using std ::endl;

class tcp_message
{
public:
	tcp_message()
	: body_len_(0)
	{
	}
	char* data()
	{
		return data_;
	}
	char *body()
	{
		return data_+header_len;
	}
	void clear()
	{
		memset(data_,0,header_len + max_body_len);
	}
	int get_header_len()
	{
		return header_len;
	}
	int body_len()
	{
		return body_len_;
	}
	int msg_len()
	{
		return header_len+body_len_;
	}
	bool set_body_len(int new_len)
	{
		body_len_ = new_len;
		if (body_len_ > max_body_len)
		{
			cout<<"body len too large , resized"<<endl;
			body_len_ = max_body_len;
		}
	}
	bool decode_header();//将报头的4字节字符串转换成数字
	void encode_header();//把数据部分大小编码成字符串  
	void decode_to_my_message(struct my_message &msg)	;  
	void encode_my_message_result(struct my_message &msg );  
private:
	enum { header_len = 4 };
	enum { max_body_len = 10240 };
	char data_[header_len + max_body_len];
	int body_len_;
};

#endif //TCP_MESSAGE_H
