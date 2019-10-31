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
	listenFd_(socket_bind_listen(port_))//注册描述符并开始监视
{
	acceptChannel_->setFd(listenFd_);
	handle_for_sigpipe();//为了避免进程退出, 可以捕获SIGPIPE信号, 或者忽略它, 给它设置SIG_IGN信号处理函数
	if (setSocketNonBlocking(listenFd_) < 0)
	{
		perror("set socket non block failed");//没有perror的头文件？
		abort();//abort的也没有
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











