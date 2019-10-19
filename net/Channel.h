#pragma once
//handleError()会用到
#include <string>
#include <memory>
#include <functional>



//头文件<memory>
typedef std::shared_ptr<Channel> SP_Channel;
class EventLoop;
class Channel
{
private:
	//头文件<functional>
	typedef std::function<void()> EventCallBack;
	EventLoop *loop_;
	int fd_;
	//无符号32位整型
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
        void getEvents()
	{
		return events_;
	}
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

	//定时器不用muduo方法
	//void handleEvent(TimeStamp receiveTime)
	void handleEvent()
	{
		//为什么events_=0 因为已经有事件被触发了 所以events_已经不重要了吗 是因为拿到事件了就不期望在这段时间内在监控任何事件
		events_ = 0 ;
		//这里是挂断并且revents_无事件？
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
	}
	
                //void handleError(int fd,int err_num,std::string short_msg);

		//muduo用enableReading更新events_,并且调用update()
		//update会在哪
	        //在epoll里面通过timerManager更新？
		void setEvents(__uint32_t ev)
		{
			events_ = ev ;
		}

		void setRevents(__uint32_t ev)
		{
			revents_ = ev ;
		}

         //在改某个fd在epoll中监听的事件时，如果和上次一模一样，那就不用改了，算是个小优化，去掉也没问题 --linya
		bool isEqualAndUpdateLastEvents()
		{
			bool ret(lastEvents_ == events_);
			lastEvents_ = events_ ;
			return ret;
		}

                __uint32_t getLastEvents()
		{
			return lastEvents_;
		}










};
