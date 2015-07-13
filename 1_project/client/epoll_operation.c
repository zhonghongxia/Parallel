#include "globle.h"

int connect_again (int sock_fd )
{
    return 0;
}
/*********************
����
����:����socketΪ��������
*********************/
int make_socket_non_blocking (int sfd)
{
	int flags, s;

	//�õ��ļ�״̬��־
	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
	  perror ("error from funtion  : fcntl ����");
	  return -1;
	}

	//�����ļ�״̬��־
	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
	  perror ("error from funtion : fcntl ����");
	  return -1;
	}
	return 0;
}


void sig_fun(int signo)
{
    int i;
    for(i = 0 ; i < CLI_NUM; i++)
    {
        if(pack_ptr[i] != NULL)
           free(pack_ptr[i]);
        printf("free num : %d success\n",i);
    }
    raise(SIGQUIT);
}
int clean (int efd ,PACK *ptr)
{
	struct epoll_event event;
	memset (&event , 0 , sizeof(struct epoll_event));
	if(ptr ->write_buf != NULL)
	{
		free (ptr ->write_buf);
		ptr ->write_buf = NULL;
	}
	if(ptr ->read_buf != NULL)
	{
		free (ptr ->read_buf);
		ptr ->read_buf = NULL;
	}
	epoll_ctl(efd , EPOLL_CTL_DEL , ptr->sock_fd , &event);
	return 0;

}
