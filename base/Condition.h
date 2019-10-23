// @file    Condition.h
// @author  axunzzz
// @date    2019/10/23
// @Email   469826336@qq.com
#pragma once
#include"noncopyable.h"
#include "MutexLock.h"
#include <pthread.h>
#include<errno.h>
#include<cstdint>
//time.h 是ISO C99 标准日期时间头文件。
//sys/time.h 是Linux 系统的日期时间头文件。
//sys/time.h 通常会包含#include<time.h>
#include<time.h>


class Condition:noncopyable
{
private:
	MutexLock mutex_;
	pthread_cond_t cond_;

public:
	explicit Condition(MutexLock &mutex)
		:mutex_(mutex)
	{
		pthread_cond_init(&cond_,NULL);
	}

	~Condition()
	{
		pthread_cond_destroy(&cond_);
	}

	void wait()
	{
		//这里会 解锁->等待条件触发->加锁
		pthread_cond_wait(&cond_,mutex_.get());
	}

	void notify()
	{
		pthread_cond_signal(&cond_);
	}

	void notifyAll()
	{
		pthread_cond_broadcast(&cond_);
	}

	bool waitForSeconds(int seconds)
	{
		struct timespec abstime;
		clock_gettime(CLOCK_REALTIME,&abstime);//CLOCK_REALTIME:系统实时时间;clock_gettime( ) 提供了纳秒级的精确度
		abstime.tv_sec += static_cast<time_t>(seconds);//到达了abstime时间，而后才解锁
		return ETIMEDOUT == pthread_cond_timewait(&cond_,mutex_.get(),&abstime);//#define ETIMEDOUT 110 /* Connection timed out */
	}




};