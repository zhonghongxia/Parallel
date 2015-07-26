//server.h
#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>
#include <netinet/in.h>     //socket
#include <unistd.h>            //gethostname
#include <arpa/inet.h>
#include <string.h>            //memset
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

#define INVALID_SOCKET   -1
#define SOCKET_ERROR     -1
#define TRUE             0
#define FALSE            -1
 
#endif