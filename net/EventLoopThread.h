// @file    EventLoopThread.h
// @author  axunzzz
// @date    2019/10/23
// @Email   469826336@qq.com
#pragma once
#include "base/noncopyable.h"
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/CountDownLatch.h"
#include "EventLoop.h"
//暂时没有头文件Thread.h

class EventLoopThread:noncopyable
{
public:
	EventLoopThread();
	~EventLoopThread();
	EventLoop* startLoop();

private:
		void threadFunc();
		EventLoop *loop_;
		bool exiting_;
		Thread thread_;
		MutexLock mutex_;
		Condition cond_;
};