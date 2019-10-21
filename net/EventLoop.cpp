#include "EventLoop.h"



//__thread修饰，每个线程都有一个独立实体
__thread EventLoop* t_loopInThisThread = 0;

EventLoop::EventLoop()
	:looping_(false),
	threadId_(CurrentThread::tid())，
	quit_(false)
{
	//one loop per thread
	if (t_loopInThisThread)
	{
		//LOG << "Another EventLoop" << t_loopInThisThread << "exists in this thread" << threadId_;
	} 
	else
	{
		t_loopInThisThread = this ;
	}
}


EventLoop::~EventLoop()
{
	//防止在loop()中
	assert(!looping_);
	t_loopInThisThread = NULL ;
}


void EventLoop::loop()
{
	assert(!looping_);
	assert(isInLoopThread());
	looping_ = true ;
        quit_ = false ;
	//LOG_TRACE << "EventLoop" << this <<"start looping";
	std::vector<SP_Channel> activeChannels;
	while (!quit_)
	{
		activeChannels.clear();
		activeChannels = poller_->poll();
		for (auto &it : activeChannels)
		{
			it->handleEvent();
		}




	}
	looping_=false ;
}







