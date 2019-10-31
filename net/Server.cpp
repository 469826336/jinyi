// @file    Server.cpp
// @author  axunzzz
// @date    2019/10/31
// @Email   469826336@qq.com
#include "Server.h"
#include<functional>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include "Util.h"
Server::Server(EventLoop* loop,int threadNum,int port)
	:loop_(loop),
	threadNum_(threadNum),
	eventLoopThreadPool_(new EventLoopThreadPool(loop_,threadNum)),
	started_(false),
	port_(port),
	listenFd_(socket_bind_listen(port_))//ע������������ʼ����
{
	acceptChannel_->setFd(listenFd_);
	handle_for_sigpipe();//Ϊ�˱�������˳�, ���Բ���SIGPIPE�ź�, ���ߺ�����, ��������SIG_IGN�źŴ�����
	if (setSocketNonBlocking(listenFd_) < 0)
	{
		perror("set socket non block failed");//û��perror��ͷ�ļ���
		abort();//abort��Ҳû��
	}
}

void Server::start()
{
	eventLoopThreadPool_->start();
	acceptChannel_->setEvents(EPOLLIN | EPOLLET);
	acceptChannel_->setReadHandle(bind(&Server::handNewConn,this));
	acceptChannel_->setConnHandler(bind(&Server::handThisConn,this));
	loop_->addToPoller(acceptChannel_,0);
	started_ = true;
}











