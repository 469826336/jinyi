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
		//һֱ�ȵ�threadFun��Thread������������
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
	//assert(exiting_) ����EventLoop�����������߳�����������������ͬ�������threadFunc()�˳�֮��
	//���ָ���ʧЧ�ˡ����ڷ������һ�㲻Ҫ���ܰ�ȫ���˳�����Ӧ�ò���ʲô������
	loop_ = NULL


}


