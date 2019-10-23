// @file    MutexLock.h
// @author  axunzzz
// @date    2019/10/22
// @Email   469826336@qq.com
#pragma once
#include"base/noncopyable.h"
#include <pthread.h>
//û������������io
//#include <cstdio>

//�޷�������ֻ�ܴ��ݶ����ָ���������
class MutexLock:noncopyable
{
public:
	MutexLock()
	{
		pthread_mutex_init(&mutex_,NULL);
	}

	~MutexLock()
	{
		//���������֮�������߳̾Ϳ�ס�ˣ��൱��ʧЧ��
		pthread_mutex_lock(&mutex_);
		//������linux�У�����������ռ���κ���Դ���������������˼����״̬���⣨����״̬�򷵻�EBUSY��û����������
		pthread_mutex_destroy(&mutex_);
	}

	void lock()
	{
		pthread_mutex_lock(&mutex_);
	}

	void unlock()
	{
		pthread_mutex_unlock(&mutex_);
	}

	//��������������õ�get() Condition::wait(),timewait()
	pthread_mutex_t *get()
	{
		return &mutex_;
	}


private:
	pthread_mutex_t mutex_;
	friend class Condition;
};

class MutexLockGuard:noncopyable
{
public:
	//����һЩ�����Ǳ�ڴ��󣬺ܶ�ʱ��һ��������ĸ�ֵ�����Ǵ�ģ����û��explicit����ֵ�����ֿ���ʽת��Ϊ�����������ͣ�Ҳ�ͳ���һ�����Է��ֵ�BUG
	explicit MutexLockGuard(MutexLock & mutex)
		:mutex_(mutex)
	{
		mutex_.lock();
	}

	~MutexLockGuard()
	{
		mutex_.unlock();
	}

private:
	MutexLock &mutex_;
};