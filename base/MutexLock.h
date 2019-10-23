// @file    MutexLock.h
// @author  axunzzz
// @date    2019/10/22
// @Email   469826336@qq.com
#pragma once
#include"base/noncopyable.h"
#include <pthread.h>
//没看到哪里用了io
//#include <cstdio>

//无法拷贝类只能传递对象的指针或者引用
class MutexLock:noncopyable
{
public:
	MutexLock()
	{
		pthread_mutex_init(&mutex_,NULL);
	}

	~MutexLock()
	{
		//这里加了锁之后其他线程就卡住了，相当于失效了
		pthread_mutex_lock(&mutex_);
		//由于在linux中，互斥锁并不占用任何资源，因此这个函数除了检查锁状态以外（锁定状态则返回EBUSY）没有其他动作
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

	//还不明白哪里会用到get() Condition::wait(),timewait()
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
	//避免一些无意的潜在错误，很多时候一个不经意的赋值本来是错的，结果没有explicit，赋值对象又可隐式转换为参数这种类型，也就成了一个难以发现的BUG
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