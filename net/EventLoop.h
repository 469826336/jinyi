#pragma once
#include "base/CurrentThread.h"
#include "MutexLock.h"
#include "Epoll.h"
#include <memory>
#include<vector>
#include<functional>
#include <sys/eventfd.h>
#include"base/MutexLock.h"
class EventLoop
{
public:	
	typedef std::function<void()> Functor;
	EventLoop();
	~EventLoop();
	void loop();
	void quit();
	void runInLoop(Functor&& cb);//右值引用提高效率，减少复制
	void queueInLoop(Functor&& cb);

	//判断是否在loop线程中
	bool isInLoopThread()const
	{
		return threadId_ == CurrentThread::tid();
	}

	void assertInLoopThread()
	{
		//没有加头文件<assert.h>
		assert(isInLoopThread());
	}


private:

	bool looping_;//atomic
	bool quit_;//atomic
	bool eventHandling_;//atomic
	const pid_t threadId_;//pid_t这个类型定义实际上就是int型
	shared_ptr<Epoll> poller_;//通过shared_ptr间接持有Poller
	//声明顺序 wakeup_ > wakeupChannel_ 不然构造函数出问题
	int wakeupFd_;//唤醒用eventfd
	shared_ptr<Channel> wakeupChannel_;
	bool callingPendingFunctors_;
	std::vector<Functor> pendingFunctors_;//暴露给了其他线程，因此用mutex_保护
	mutable MutexLock mutex_;


	void wakeup();
	void handleRead();
	void doPendingFunctors();



}