#pragma once
//handleError()���õ�
#include <string>
#include <memory>
#include <functional>



//ͷ�ļ�<memory>
typedef std::shared_ptr<Channel> SP_Channel;
class EventLoop;
class Channel
{
private:
	//ͷ�ļ�<functional>
	typedef std::function<void()> EventCallBack;
	EventLoop *loop_;
	int fd_;
	//�޷���32λ����
	__uint32_t events_;
	__uint32_t revents_;
	EventCallBack readHandler_;
	EventCallBack writeHandler_;
	EventCallBack errorHandler_;

public:
	Channel(EventLoop *loop);
	Channel(EventLoop *loop,int fd);
	~Channel();
	int getFd();
	void setFd(int fd);

	void setReadHandle(EventCallBack readHandler)
	{
		readHandler_ = readHandler ;
	}

	void setWriteHandle(EventCallBack writeHandler)
	{
		writeHandler_ = writeHandler ;
	}

	void setErrorHandle(EventCallBack errorHandler)
	{
		errorHandler_ = errorHandler ;
	}

	//��ʱ������muduo����
	//void handleEvent(TimeStamp receiveTime)
	void handleEvent()
	{
		//Ϊʲôevents_=0 ��Ϊ�Ѿ����¼��������� ����events_�Ѿ�����Ҫ����
		events_ = 0 ;
		//�����ǹҶϲ���revents_���¼���
		if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
		{
			return;
		}
		
		if (revents_ & EPOLLERR)
		{
			if (errorHandler_)
			{
				errorHandler_();
			}
		}

		if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
		{
			if (readHandler_)
			{
				readHandler_();
			}
		}

		if (revents_ & EPOLLOUT)
		{
			if (writeHandler_)
			{
				writeHandler_();
			}
		}

		//void handleError(int fd,int err_num,std::string short_msg);

		//muduo��enableReading����events_,���ҵ���update()
		//update������
		void setEvents(__uint32_t ev)
		{
			events_ = ev ;
		}

		void setRevents(__uint32_t ev)
		{
			revents_ = ev ;
		}










	}












};