// @file    EventLoopThreadPool.cpp
// @author  axunzzz
// @date    2019/10/29
// @Email   469826336@qq.com
#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop,int threadNum)
	:baseloop_(baseloop),
	started_(false),
	threadNums_(threadNum),
	next_(0)
{
	if (threadNums_ <= 0)
	{

		abort();
	}
}

void EventLoopThreadPool::start()
{
	baseloop_->assertInLoopThread()	;
	started_ = true;
	for (int i = 0;i < threadNums_;++i)
	{
		std::shared_ptr<EventLoopThread> t(new EventLoopThread());
		threads_.push_back(t);
		loops_.push_back(t->startLoop());
	}
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
	baseloop_->assertInLoopThread();
	//没有assert.h?
	assert(started_);
	EventLoop* loop = baseloop_;
	//判断单线程或多线程，round-robin算法，把每一次来自用户的请求轮流分配
	if (!loops_.empty())
	{
		loop = loops_[next_];
		next_ = (next_ + 1) % threadNums_;
	}
	return loop;
}