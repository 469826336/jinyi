// @file    CountDownLatch.h
// @author  axunzzz
// @date    2019/10/24
// @Email   469826336@qq.com
#pragma once
#include "noncopyable.h"
#include "Condition.h"
#include "MutexLock.h"


//CountDownLatch主要作用是确保Thread中传进去的func真的启动了以后
//外层的start才返回，不然func要是没有启动，EventLoopThread::startLoop()
//中cond_.wait将一直等待
class CountDownLatch:noncopyable
{
public:
	explicit CountDownLatch(int count);
	void wait();
	void countDown();

private:
	//mutable有什么作用？
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};