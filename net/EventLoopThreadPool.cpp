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
	//û��assert.h?
	assert(started_);
	EventLoop* loop = baseloop_;
	//�жϵ��̻߳���̣߳�round-robin�㷨����ÿһ�������û���������������
	if (!loops_.empty())
	{
		loop = loops_[next_];
		next_ = (next_ + 1) % threadNums_;
	}
	return loop;
}