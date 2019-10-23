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
	void runInLoop(Functor&& cb);//��ֵ�������Ч�ʣ����ٸ���
	void queueInLoop(Functor&& cb);

	//�ж��Ƿ���loop�߳���
	bool isInLoopThread()const
	{
		return threadId_ == CurrentThread::tid();
	}

	void assertInLoopThread()
	{
		//û�м�ͷ�ļ�<assert.h>
		assert(isInLoopThread());
	}


private:

	bool looping_;//atomic
	bool quit_;//atomic
	bool eventHandling_;//atomic
	const pid_t threadId_;//pid_t������Ͷ���ʵ���Ͼ���int��
	shared_ptr<Epoll> poller_;//ͨ��shared_ptr��ӳ���Poller
	//����˳�� wakeup_ > wakeupChannel_ ��Ȼ���캯��������
	int wakeupFd_;//������eventfd
	shared_ptr<Channel> wakeupChannel_;
	bool callingPendingFunctors_;
	std::vector<Functor> pendingFunctors_;//��¶���������̣߳������mutex_����
	mutable MutexLock mutex_;


	void wakeup();
	void handleRead();
	void doPendingFunctors();



}