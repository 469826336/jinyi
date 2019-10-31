// @file    EventLoopThread.cpp
// @author  axunzzz
// @date    2019/10/24
// @Email   469826336@qq.com
#include"EventLoop.h"
#include <functional>//bind()


EventLoopThread::EventLoopThread()
	:loop_(NULL),
	exiting_(false),
	thread_(bind(&EventLoopThread::threadFunc,this),"EventLoopThread"),
	mutex_(),
	cond_(mutex_)
{ }


EventLoop* EventLoopThread::~EventLoopThread()
{
	exiting_ = true;
	if (loop_ != NULL)
	{
		loop_->quit();
		thread_.join();
	}
	return loop_;
}


void EventLoopThread::startLoop()
{
	assert(!thread_.started());
	thread_.start();
	{
		MutexLockGuard lock(mutex_);
		//一直等到threadFun在Thread里真正跑起来
		while (loop_ == NULL)
		{
			cond_.wait();
		}
		return loop_;
	}
}


void EventLoopThread::threadFunc()
{
	EventLoop loop;

	{
		MutexLockGuard lock(mutex_);
		loop_ = &loop;
		cond_.notify();
	}

	loop.loop();
	//assert(exiting_) 由于EventLoop的生命期与线程主函数的作用域相同，因此在threadFunc()退出之后
	//这个指针就失效了。好在服务程序一般不要求能安全地退出，这应该不是什么大问题
	loop_ = NULL


}


