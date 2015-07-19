#ifndef SESSION_H
#define SESSION_H
#include <boost/shared_ptr.hpp>
#include "tcpmessage.h"
#include "mymessage.h"
using boost::asio::ip::tcp;  
class session
{
public:
    session(boost::asio::io_service& io_service)
        : socket_(io_service)
    {

    }

    virtual ~session()
    {
//        boost::mutex::scoped_lock lock(cout_mtx);
    }

    tcp::socket& socket()
    {
        return socket_;
    }
	void start()
	{
		tcp_msg_.clear();
            boost::asio::async_read(socket_, boost::asio::buffer(tcp_msg_.data(), tcp_msg_.get_header_len()),
                boost::bind(&session::h_read_body, this, 
				boost::asio::placeholders::error));
	
	}
	void h_read_header(const boost::system::error_code& error)
	{        
		if(!error)
        {
		tcp_msg_.clear();
            boost::asio::async_read(socket_, boost::asio::buffer(tcp_msg_.data(), tcp_msg_.get_header_len()),
                boost::bind(&session::h_read_body, this, 
				boost::asio::placeholders::error));
        }
        else 
        {
            delete this;
        
        }
	}
	void h_read_body(const boost::system::error_code& error)
    {
        if (!error)
        {	
			tcp_msg_.decode_header();
            boost::asio::async_read(socket_, boost::asio::buffer(tcp_msg_.body(), tcp_msg_.body_len()),
                boost::bind(&session::h_pic_match, this, 
				boost::asio::placeholders::error));
        }
        else
        {
            delete this;
        }
    }
	void h_pic_match(const boost::system::error_code& error)
    {
		
        if (!error)
        {	
			tcp_msg_.decode_to_my_message(my_msg_);
            //进行match的调用
			my_msg_.print();
            //
            //
            //            
            query_db();
			//
			//
			//
			//
			sent_result_back();
        }
        else
        {
            delete this;
        }
    }
    void query_db(/*  */)
    {
        //进行数据库的操作  
		//返回的结果记录在my_msg_中		
		my_msg_.result_len = 100;
		my_msg_.result = new char[my_msg_.result_len];
		memset(my_msg_.result , 0 , 100);
		sprintf(my_msg_.result , "%s" , "/^_^/1234" );
        tcp_msg_.clear();
		tcp_msg_.encode_my_message_result(my_msg_);
		delete []my_msg_.result;
		
    }	
	void sent_result_back(/*  */)
    { 
         boost::asio::async_write(socket_, boost::asio::buffer(tcp_msg_.data(),tcp_msg_.msg_len()),
            boost::bind(&session::h_read_header, this, 
			boost::asio::placeholders::error));

    }
private:
	my_message 	my_msg_; 
	tcp_message tcp_msg_;
    tcp::socket socket_;   
};
#endif
