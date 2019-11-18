// @file    EventLoopThreadPool.h
// @author  axunzzz
// @date    2019/10/29
// @Email   469826336@qq.com
#pragma once
#include "base/noncopyable.h"
#include "EventLoopThread.h"
#include <memory>
#include <vector>

class EventLoopThreadPool:noncopyable
{
public:
	EventLoopThreadPool(EventLoop* baseloop,int threadNums);

	~EventLoopThreadPool()
	{
		
	}
	void start();

	EventLoop *getNextLoop();

private:
	EventLoop* baseloop_;
	bool started_;
	int threadNums_;
	int next_;
	std::vector<std::shared_ptr<EventLoopThread>> threads_;
	std::vector<EventLoop*>loops_;
};
