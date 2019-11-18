// @file    Util.cpp
// @author  axunzzz
// @date    2019/10/31
// @Email   469826336@qq.com
#include "Util.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>//不知道这个用来干嘛
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


// 返回已经开始监听的描述符，如果初始化失败返回-1
int socket_bind_listen(int port)
{
	// 检查port值，取正确区间范围,在TCP、UDP协议的开头，会分别有16位来存储源端口号和目标端口号，所以端口个数是2^16-1=65535个
	if (port < 0 || port > 65535)
	{
		return -1;
	}

	// 创建socket(IPv4 + TCP),返回监听描述符
	int listen_fd = 0;
	if ((listen_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		close(listen_fd);
		return -1;
	}

	// 消除bind时"Address already in use"错误
	int optVal = 1;
	if (setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&optVal,sizeof(optVal)) == -1)
	{
		close(listen_fd);
		return -1;
	}

	// 设置服务器IP、Port和监听描述符绑定
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

	// 开始监听，最大等待队列长为LISTENQ
	if (listen(listen_fd,2048) == -1)
	{
		close(listen_fd);
		return -1;
	}

	//无效监听描述符
	if (listen_fd == -1)
	{
		close(listen_fd);
		return -1;
	}

	return listen_fd;
}

void handle_for_sigpipe()
{
	struct sigaction sa;//头文件signal.h
	memset(&sa,'\0',sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if (sigaction(SIGPIPE,&sa,NULL))//现在大多系统都用sigaction重新实现了signal函数
	{
		return;//有什么意义呢？
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

void setSocketNodelay(int fd)
{
	int enable = 1;
	setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(void*)&enable,sizeof(enable));
}


const int MAX_BUFF = 4096;
ssize_t readn(int fd,std::string &inBuffer,bool &zero)
{
	ssize_t nread = 0;
	ssize_t readSum = 0;
	while (true)
	{
		char buff[MAX_BUFF];
		if ((nread = read(fd,buff,MAX_BUFF)) < 0)
		{
			if (errno == EINTR)
			{
				continue;
			}
			else if (errno == EAGAIN)
			{
				return readSum;
			}
			else 
			{
				perror("read error");
				return -1;
			}
		}
		else if (nread == 0)
		{
			zero = true;
			break;
		}
		readSum += nread;
		inBuffer += std::string(buff,buff+nread);
	}
	return readSum;
}

ssize_t writen(int fd,void *buff,size_t n)
{
	size_t nleft = n;
	ssize_t nwritten = 0;
	ssize_t writeSum = 0;
	char *ptr = (char*)buff;
	while(nleft > 0)
	{
		if ((nwritten = write(fd,ptr,nleft)) <= 0)
		{
			if (nwritten < 0)
			{
				if (errno == EINTR)
				{
					nwritten = 0;
					continue;
				}
				else if (errno ==EAGAIN)
				{
					return writeSum;
				}
				else
					return -1;
			}
		}
		writeSum +=nwritten;
		nleft -= nwritten;
		ptr += nwritten;
	}
	return writeSum;
}

ssize_t writen(int fd,std::string &sbuff)
{
	size_t nleft = sbuff.size();
	ssize_t nwritten = 0;
	ssize_t writeSum = 0;
	const char* ptr = sbuff.c_str();
	while (nleft > 0)
	{
		if ((nwritten = write(fd,ptr,nleft)) <= 0)
		{
			if (nwritten < 0)
			{
				if (errno == EINTR)
				{
					nwritten = 0;
					continue;
				}
				else if (errno == EAGAIN)
				{
					break;
				}
				else 
					return -1;
			}
		}
		writeSum += nwritten;
		nleft -= nwritten;
		ptr += nwritten;
	}
	if (writeSum == static_cast<int>(sbuff.size()))
	{
		sbuff.clear();
	} 
	else
	{
		sbuff = sbuff.substr(writeSum);
	}
	return writeSum;
}

void shutDownWR(int fd)
{
	shutdown(fd,SHUT_WR);
}
