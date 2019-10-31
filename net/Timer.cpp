// @file    Timer.cpp
// @author  axunzzz
// @date    2019/10/21
// @Email   469826336@qq.com
#include "Timer.h"
#include<sys/time.h>




Timer::Timer(std::shared_ptr<HttpData> requestData,int timeout)
	:deleted_(false),
	SPHttpData(requestData)
{
	struct timeval now;
	gettimeofday(&now,NULL);
	//以毫秒计,tv_sec为什么要%10000
	expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}


Timer::~Timer()
{
	if (SPHttpData)
	{
		SPHttpData->handleClose();
	}
}


//这里的复制构造没有复制deleted_?
Timer::Timer(Timer &tn)
	:SPHttpData(tn.SPHttpData),
	expiredTime_(0)
{}


void Timer::update(int timeout)
{
	struct timeval now;
	gettimeofday(&now,NULL);
	expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}


bool Timer::isValid()
{
	struct timeval now;
	gettimeofday(&now,NULL);
	size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
	if (temp < expiredTime_)
	{
		return true;
	} 
	else
	{
		this->setDeleted();
		return false;
	}
}


void Timer::clearReq()
{
	SPHttpData->reset();
	this->setDeleted();
}


TimerQueue::TimerQueue()
{}


TimerQueue::~TimerQueue()
{}


void TimerQueue::addTimer(std::shared_ptr<HttpData> SPHttpData,int timeout)
{
	SP_Timer new_timer(new Timer(SPHttpData,timeout));
	timerQueue.push(new_timer);
	//linkTimer还没看
	SPHttpData->linkTimer(new_timer)
}


void TimerQueue::handleExpiredTimer()
{
	while (!timerQueue.empty())
	{
		SP_Timer tmpSPTimer = timerQueue.top();
		if (tmpSPTimer->isDeleted())
		{
			timerQueue.pop();
		} 
		else if (tmpSPTimer->isValid() == false)
		{
			timerQueue.pop();
		}
		else
		{
			break;
		}
	}
}
