#pragma once
#include "base/CurrentThread.h"

//首先定义EventLoop class 基本接口:构造函数、析构函数、loop()成员函数，（改进：class不可复制）
class EventLoop
{
public:	
	EventLoop();
	~EventLoop();

	//时间循环
	void loop();

	//判断是否在loop线程中
	bool isInLoopThread()const
	{
		return threadId_ == CurrentThread::tid();
	}

	void assertInLoopThread()
	{
		//没有加头文件<assert.h>
		assert(isInLoopThread());
	}


private:
	bool looping_;//atomic
	const pid_t threadId_;//pid_t这个类型定义实际上就是int型





}
