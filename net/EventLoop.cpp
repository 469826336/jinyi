#include "EventLoop.h"



//__thread修饰，每个线程都有一个独立实体
__thread EventLoop* t_loopInThisThread = 0;


int createEventfd()
{
	int evtfd = eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtfd < 0)
	{
		LOG << "Failed in eventfd";
		abort();
	}
	return evtfd;
}
EventLoop::EventLoop()
	:looping_(false),
	threadId_(CurrentThread::tid())，
	quit_(false),
	eventHandling_(false),
	poller_(new Epoll()),
	wakeupFd_(CreateEventfd()),
	callingPendingFunctors_(false),
	wakeupChannel_(new Channel(this,wakeupFd_))
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

	wakeupChannel_->setEvents(EPOLLIN | EPOLLET);//EPOLLET将EPOLL设为边缘触发(Edge Triggered)模式
	wakeupChannel_->setReadHandle(bind(&EventLoop::handleRead,this));
	poller_->epoll_add(wakeupChannel_,0);
}


EventLoop::~EventLoop()
{
	//防止在loop()中
	assert(!looping_);
	t_loopInThisThread = NULL ;
}


void EventLoop::wakeup()
{
	uint64_t one = 1;
	ssize_t n = writen(wakeupFd_,(char*)(&one),sizeof one);
	//sizeof(one)?
	if (n != sizeof one)
	{
		LOG << "EventLoop::wakeup() writes" << n << "bytes instead of 8";
	}
}


void EventLoop::handleRead()
{
	uint64_t one = 1;
	ssize_t n = readn(wakeupFd_,&one,sizeof one);
	if (n != sizeof one)
	{
		LOG << "EventLoop::handleRead() reads" << n << "bytes instead of 8";
	}
	//每次handleEvents时都会把events置为0
	wakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}
void EventLoop::runInLoop(Functor&& cb)
{
	if (isInLoopThread())
	{
		cb();
	} 
	else
	{
		queueInLoop(std::move(cb));//为什么这里还要再转一次?
	}
}


void EventLoop::queueInLoop(Functor&& cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.emplace_back(std::move(cb));//cb不已经是右值引用了吗 为什么还要move()
	}
	if (!isInLoopThread() || callingPendingFunctors_)
	{
		wakeup();
	}
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
		eventHandling_ = true;
		for (auto &it : activeChannels)
		{
			it->handleEvent();
		}
		eventHandling_ = false;
		doPendingFunctors();
		poller_->handleExpired();


	}

	looping_=false ;
}


void EventLoop::doPendingFunctors()
{
	std::vector<Functor> functors;
	callingPendingFunctors_ = true;
	
	{
		MutexLockGuard lock(mutex_);
		//EventLoop::doPendingFunctors()不是简单地在临界区内依次调用Functor，而是把
		//回调列表swap()到局部变量functors中，这样一方面减小了临界区的长度(意味着不会
		//阻塞其他线程调用queueInLoop())，另一方面也避免了死锁(因为Functor可能再调用
		//queueInLoop())
		//一方面大括号使锁的范围减小，另一方面避免了死锁
		functors.swap(pendingFunctors_);
	}

	for (const Functor& functor : functors)
	{
		functor();
	}
	callingPendingFunctors_ = false;
}


void EventLoop::quit()
{
	quit_ = true;
	if (!isInLoopThread())
	{
		wakeup();
	}
}




