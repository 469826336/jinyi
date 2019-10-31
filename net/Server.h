// @file    Server.h
// @author  axunzzz
// @date    2019/10/31
// @Email   469826336@qq.com
#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "EventLoopThreadPool.h"
#include <memory>


class Server
{
public:
	Server(EventLoop *loop,int threadNum,int port);
	~Server();
	EventLoop* getLoop() const { return loop_; }
	void start();
	void handNewConn();
	void handThisConn() { loop_->updatePoller(acceptChannel_);}

private:
	EventLoop* loop_;
	int threadNum_;
	std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
	bool started_;//这个是并发模型里面提到的锁吗？
	std::shared_ptr<Channel> acceptChannel_;
	int port_;
	int listenFd_;
	static const int MAXFDS = 10000;
};











