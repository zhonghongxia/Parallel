//protocol.h 
//约定:
//网络中所有文件和url的传输，都不以\0作为结尾，而是具体长度,memset系列函数进行
//对数字的传输，不用atoi等函数，直接传输，然后强转
#ifndef PROTOCOL_H
#define PROTOCOL_H

#define KEEPON_LEN 4
#define FILECONTENTLEN_LEN 4
#define FILENAME_LEN 20 //这里后期直接用FILEID来表示filename避免不必要的字节浪费
#define SOURCEID_LEN 4

#endif 