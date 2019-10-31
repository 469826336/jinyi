// @file    Util.cpp
// @author  axunzzz
// @date    2019/10/31
// @Email   469826336@qq.com
#include "Util.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>//��֪�������������
#include <string.h>
#include <signal.h>
#include <fcntl.h>

// �����Ѿ���ʼ�������������������ʼ��ʧ�ܷ���-1
int socket_bind_listen(int port)
{
	// ���portֵ��ȡ��ȷ���䷶Χ,��TCP��UDPЭ��Ŀ�ͷ����ֱ���16λ���洢Դ�˿ںź�Ŀ��˿ںţ����Զ˿ڸ�����2^16-1=65535��
	if (port < 0 || port > 65535)
	{
		return -1;
	}

	// ����socket(IPv4 + TCP),���ؼ���������
	int listen_fd = 0;
	if ((listen_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		close(listen_fd);
		return -1;
	}

	// ����bindʱ"Address already in use"����
	int optVal = 1;
	if (setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&optVal,sizeof(optVal)) == -1)
	{
		close(listen_fd);
		return -1;
	}

	// ���÷�����IP��Port�ͼ�����������
	struct sockaddr_in server_addr;
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons((unsigned short)port);
	if (bind(listen_fd,(struct sockaddr*)&server_addr,sizeof(server_addr)) == -1)
	{
		close(listen_fd);
		return -1;
	}

	// ��ʼ���������ȴ����г�ΪLISTENQ
	if (listen(listen_fd,2048) == -1)
	{
		close(listen_fd);
		return -1;
	}

	//��Ч����������
	if (listen_fd == -1)
	{
		close(listen_fd);
		return -1;
	}

	return listen_fd;
}

void handle_for_sigpipe()
{
	struct sigaction sa;//ͷ�ļ�signal.h
	memset(&sa,'\0',sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if (sigaction(SIGPIPE,&sa,NULL))//���ڴ��ϵͳ����sigaction����ʵ����signal����
	{
		return;//��ʲô�����أ�
	}
}

int setSocketNonBlocking(int fd)
{
	int flag = fcntl(fd,F_GETFL,0);
	if (flag == -1)
	{
		return -1;
	}
	flag |= 0_NONBLOCK;
	if (fcntl(fd,F_SETFL,flag) == -1)
	{
		return -1;
	}
	return 0;
}












