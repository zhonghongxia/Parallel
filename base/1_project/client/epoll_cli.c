#include "globle.h"

int count = 0;
PACK ** pack_ptr ;
int
main(int argc, char **argv)
{
	int i,j,s,nleft;
	int epoll_fd;  
	FILE * copy_fd; 
	int cnt_fail = 0,chat_num;
float  time_use[ CLI_NUM ][ECHO_TIMES];
struct timeval start[CLI_NUM][ECHO_TIMES];
struct timeval end[CLI_NUM][ECHO_TIMES];
struct timeval con_start[CLI_NUM];
struct timeval con_end[CLI_NUM];
float con_time[CLI_NUM]; 

	struct sockaddr_in	servaddr;
	struct epoll_event events[CLI_NUM];
    struct epoll_event event;
    PACK * ptr;
	PACK pack[CLI_NUM];//定义了一个指针数组，便于今后数据的添加
    memset(&event ,0,sizeof(struct epoll_event));	
    memset (pack ,0, CLI_NUM * sizeof(CLI_NUM));
    if(signal(SIGINT , sig_fun ) == SIG_ERR)
    {
        printf("sig_func err\n");
    }
    if(signal(SIGPIPE,SIG_IGN) == SIG_ERR );
    {
        printf("SIGPIPE ignore err\n");
    }
    epoll_fd = epoll_create(CLI_NUM);
	if (argc != 2)
	{
		printf("argc error \n");
		return 1;
	}
	for (i = 0; i < CLI_NUM; i++) 
	{
        gettimeofday(&con_start[i],NULL);
		pack[i].sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		pack[i].num = i;

		pack[i].write_buf = (char *)calloc(1 ,BODY_LEN+HEAD_LEN);	
		*(int *)pack[i].write_buf = BODY_LEN;
		*(int *)(pack[i].write_buf+HEAD_LEN) = 1;
		*(int *)(pack[i].write_buf+HEAD_LEN + 4) = 9;
		*(int *)(pack[i].write_buf+HEAD_LEN + 8) = i;
		sprintf(pack[i].write_buf+HEAD_LEN+12 , "pppic----jpg-------" );
		sprintf(pack[i].write_buf+HEAD_LEN+32 , "%s" , "12345678");
		pack[i].write_ptr = pack[i].write_buf;

		pack[i].read_buf = (char *)calloc(1 ,124);			
		pack[i].read_ptr = pack[i].read_buf;

        pack[i].write_times = 0;
		pack[i].echo_times = 0;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SERV_PORT);
		inet_pton(AF_INET, argv[1], &servaddr.sin_addr);		
		s = connect(pack[i].sock_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));
		if(s == 0)
        {
			printf("sock_buf : %d connect success\n",pack[i].sock_fd);
		}       
		if(s == -1)
		{
			printf("connect 返回 -1 sock_buf : %d , i: %d \n",pack[i].sock_fd,i);
            perror("");
			close (pack[i].sock_fd);
            printf("close sock_buf : %d \n",pack[i].sock_fd );
			free (pack[i].write_buf);
			free (pack[i].read_buf);
			pack[i].write_buf = NULL ;
			pack[i].read_buf = NULL;
			cnt_fail ++;
			continue;
		}
        gettimeofday(&con_end[i],NULL);
        con_time[i]=(con_end[i].tv_sec-con_start[i].tv_sec)*1000000+(con_end[i].tv_usec-con_start[i].tv_usec);
            
		if(0 != make_socket_non_blocking (pack[i].sock_fd)) 
			printf("make_socket_non_blocking error\n");
		event.events = EPOLLOUT;
        event.data.ptr = &pack[i];
        epoll_ctl(epoll_fd , EPOLL_CTL_ADD , pack[i].sock_fd , &event);
	}
    int epoll_ready_num;
	while(count < CLI_NUM)
	{
		epoll_ready_num = epoll_wait(epoll_fd , events, CLI_NUM ,20);
		for(i = 0; i < epoll_ready_num ;i++)
		{
			if(events[i].events & EPOLLOUT)			
            {
				ptr = (PACK *)events[i].data.ptr;
				nleft = &ptr->write_buf[BODY_LEN+HEAD_LEN] - ptr->write_ptr;
#ifdef ANNOUNCE_DEGUB
				if(nleft <= 0 )
				{
					printf("temp_buf 居然<= 0 这是不该出现的\n");
				}
#endif
                if(nleft == BODY_LEN+HEAD_LEN)
                    gettimeofday(&start[ptr->num][ptr->write_times],NULL);
				//将数据写出去
                int nwritten;
				nwritten = write (ptr -> sock_fd , ptr->write_ptr ,nleft);//+++++++++++++这里可以判断一下
				if(nwritten <= 0)
				{
					if(errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
					{
						nwritten = 0;
					}
					else 
					{
						printf("epoll write error ,socket : %d \n " ,((PACK *)events[i].data.ptr)-> sock_fd);
						perror("");
					}
				}
				//修改PACK数据结构里面的数据
				else
				{
					ptr->write_ptr += nwritten;
					if(nwritten == nleft)
					{
                        ptr->write_times++;
						ptr->write_ptr = ptr->write_buf;
						event.events = EPOLLIN;
						event.data.ptr = ptr;
						if(-1 == epoll_ctl(epoll_fd , EPOLL_CTL_MOD ,ptr->sock_fd ,&event))
						{
							printf("chang mod err %d\n",ptr->sock_fd);
							clean(epoll_fd , ptr);
						}
					}
				}
			}
			//如果是可读
			if(events[i].events & EPOLLIN)
			{
				//将数据全部读了
				ptr = (PACK *)events[i].data.ptr;
                int nread;
				while(( nread = read ( ptr -> sock_fd ,ptr ->read_ptr, &ptr->read_buf[124] - ptr ->read_ptr )) > 0 )
                {
					ptr ->read_ptr += nread;
				}
				if(nread < 0)
                {
                    if(errno ==  EAGAIN || errno == EWOULDBLOCK ||  errno == EINTR)
				    {
					//  perror("资源暂时不可用\n");                   
				    }
                    else
                    {
                        perror("nread return -1\n");
						clean (epoll_fd , ptr);
                    }
                }
		if( ptr -> read_ptr  - ptr -> read_buf == 124)
                {
			printf(":%s\n",ptr -> read_buf+HEAD_LEN);
                    gettimeofday(&end[ptr->num][ptr->echo_times],NULL);
                    time_use[ptr->num][ptr->echo_times]=(end[ptr->num][ptr->echo_times].tv_sec-start[ptr->num][ptr->echo_times].tv_sec)*1000000+(end[ptr->num][ptr->echo_times].tv_usec-start[ptr->num][ptr->echo_times].tv_usec)
                            ;//微秒         
                    ptr->read_ptr = ptr -> read_buf;
					ptr->echo_times ++;
                    if(ptr -> echo_times == ECHO_TIMES/2)
                    {
                        printf("%d echo %d times \n",ptr->sock_fd,ptr->echo_times);
                    }
					if(ptr -> echo_times == ECHO_TIMES)
					{
						count ++;
                        printf("%d echo over (%d times)\n",ptr->sock_fd,ptr->echo_times);
						clean(epoll_fd ,ptr);
                        continue;
					}
                    
					event.events = EPOLLOUT;
					event.data.ptr = ptr;
					if(-1 == epoll_ctl (epoll_fd , EPOLL_CTL_MOD , ptr->sock_fd , &event))
					{
						printf("chang mod err%d\n",ptr->sock_fd);
						clean (epoll_fd , ptr);
					}
				}
			}
		}
	}
    printf("*****write file*****\n");
	if((copy_fd = fopen(copy_file_name,"w") ) == NULL)
	{
		printf("creat file :%s fail.\n",copy_file_name);
		return 1;
	}
	float total =0 ;
    float con_total =0 ;
    float min = time_use[0][0];
    float max = time_use[0][0];
    float avrg = 0;
    float min_r = con_time[0];
    float max_r = con_time[0];
    float avrg_r = 0;
	for (i = 0; i < CLI_NUM; i++) 
	{
        fprintf(copy_fd ,"num : %d us,con_time : %f us \ntime_use :\n",i,con_time[i]);
        for(j = 0 ; j < ECHO_TIMES ; j++)
        {
            fprintf(copy_fd ," %f ",time_use[i][j]);
            total += time_use[i][j];
            if(min > time_use[i][j])
                min = time_use[i][j];
            if(max < time_use[i][j])
                max = time_use[i][j];
        }
        if(min_r > con_time[i])
            min_r = con_time[i];
        if(max_r < con_time[i])
            max_r = con_time[i];
        con_total += con_time[i];
	}
    avrg = total / (CLI_NUM-cnt_fail) / ECHO_TIMES;
    avrg_r = con_total /CLI_NUM;
    printf("process time per client each time : %d ,BODY_LEN : %d min %.2fms max %.2fms average : %.2fms \n",CLI_NUM , BODY_LEN , min/1000 ,max/1000 ,avrg/1000); 
    printf("connetion time  min_r %.2f max_r %.2f avrg_r %.2f \n",min_r/1000,max_r/1000 , avrg_r/1000);
        
	fclose (copy_fd);
	return 0;
}
