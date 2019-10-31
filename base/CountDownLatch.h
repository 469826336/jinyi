// @file    CountDownLatch.h
// @author  axunzzz
// @date    2019/10/24
// @Email   469826336@qq.com
#pragma once
#include "noncopyable.h"
#include "Condition.h"
#include "MutexLock.h"


//CountDownLatch��Ҫ������ȷ��Thread�д���ȥ��func����������Ժ�
//����start�ŷ��أ���ȻfuncҪ��û��������EventLoopThread::startLoop()
//��cond_.wait��һֱ�ȴ�
class CountDownLatch:noncopyable
{
public:
	explicit CountDownLatch(int count);
	void wait();
	void countDown();

private:
	//mutable��ʲô���ã�
	mutable MutexLock mutex_;
	Condition condition_;
	int count_;
};