// @file    Timer.h
// @author  axunzzz
// @date    2019/10/21
// @Email   469826336@qq.com
#pragma once
#include "HttpData.h"
#include <memory>
#include <queue>
#include <deque>
class Timer
{
private:
	bool deleted_;
	size_t expiredTime_;
	std::shared_ptr<HttpData> SPHttpData;//类型名加_ eg：SP_HttpData 对象名不加

public:
	Timer(std::shared_ptr<HttpData> requestData,int timeout);
	~Timer();
	//不懂这个构造函数
	Timer(Timer &tn);
	void update(int timeout);
	void clearReq();
	void setDeleted(){ deleted_ = true ;}
	bool isValid();
	bool isDeleted() const { return deleted_; }
	size_t getExpTime() const { return expiredTime_; }
};


struct TimerCmp
{
	bool operator()(std::shared_ptr<Timer> &a,std::shared_ptr<Timer> &b)
	{
		return a->getExpTime() > b->getExpTime();
	}
};



class TimerQueue
{
private:
	typedef std::shared_ptr<Timer> SP_Timer;
	std::priority_queue<SP_Timer,std::deque<SP_Timer>,TimerCmp> timerQueue;
	//MutexLock lock //TimerQueue的成员函数只能在其所属的IO线程调用，因此不必加锁

public:
	TimerQueue();
	~TimerQueue();
	void addTimer(std::shared_ptr<HttpData> SPHttpData,int timeout);
	void handleExpiredTimer();


};