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
//time.h ��ISO C99 ��׼����ʱ��ͷ�ļ���
//sys/time.h ��Linux ϵͳ������ʱ��ͷ�ļ���
//sys/time.h ͨ�������#include<time.h>
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
		//����� ����->�ȴ���������->����
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
		clock_gettime(CLOCK_REALTIME,&abstime);//CLOCK_REALTIME:ϵͳʵʱʱ��;clock_gettime( ) �ṩ�����뼶�ľ�ȷ��
		abstime.tv_sec += static_cast<time_t>(seconds);//������abstimeʱ�䣬����Ž���
		return ETIMEDOUT == pthread_cond_timewait(&cond_,mutex_.get(),&abstime);//#define ETIMEDOUT 110 /* Connection timed out */
	}




};