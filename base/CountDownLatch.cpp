// @file    CountDownLatch.cpp
// @author  axunzzz
// @date    2019/10/24
// @Email   469826336@qq.com
#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
	:mutex_(),
	condition_(mutex_),
	count_(count)
{ }

void CountDownLatch::wait()
{
	MutexLockGuard lock(mutex_);
	while (count_ > 0)
	{
		condition_.wait();
	}
}

void CountDownLatch::countDown()
{
	MutexLockGuard lock(mutex_);
	--count_;
	if (count_ == 0)
	{
		condition_.notifyAll();
	}
}


