#include "EventLoop.h"



//__thread���Σ�ÿ���̶߳���һ������ʵ��
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
	threadId_(CurrentThread::tid())��
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

	wakeupChannel_->setEvents(EPOLLIN | EPOLLET);//EPOLLET��EPOLL��Ϊ��Ե����(Edge Triggered)ģʽ
	wakeupChannel_->setReadHandle(bind(&EventLoop::handleRead,this));
	poller_->epoll_add(wakeupChannel_,0);
}


EventLoop::~EventLoop()
{
	//��ֹ��loop()��
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
	//ÿ��handleEventsʱ�����events��Ϊ0
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
		queueInLoop(std::move(cb));//Ϊʲô���ﻹҪ��תһ��?
	}
}


void EventLoop::queueInLoop(Functor&& cb)
{
	{
		MutexLockGuard lock(mutex_);
		pendingFunctors_.emplace_back(std::move(cb));//cb���Ѿ�����ֵ�������� Ϊʲô��Ҫmove()
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
		//EventLoop::doPendingFunctors()���Ǽ򵥵����ٽ��������ε���Functor�����ǰ�
		//�ص��б�swap()���ֲ�����functors�У�����һ�����С���ٽ����ĳ���(��ζ�Ų���
		//���������̵߳���queueInLoop())����һ����Ҳ����������(��ΪFunctor�����ٵ���
		//queueInLoop())
		//һ���������ʹ���ķ�Χ��С����һ�������������
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




