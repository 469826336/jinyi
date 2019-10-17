#pragma once
#include "base/CurrentThread.h"

//���ȶ���EventLoop class �����ӿ�:���캯��������������loop()��Ա���������Ľ���class���ɸ��ƣ�
class EventLoop
{
public:	
	EventLoop();
	~EventLoop();

	//ʱ��ѭ��
	void loop();

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
	const pid_t threadId_;//pid_t������Ͷ���ʵ���Ͼ���int��





}