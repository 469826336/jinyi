#include "Epoll.h"
#include <error.h>
//socket.h在哪用到
//#include <sys/socket.h>
//两个与tcp/ip有关的头文件
//#include <netinet/in.h>
//#include <arpa/inet.h>
//也没看到queue和deque
#include<assert.h>
//也没看到iostream


const int EVENTNUM = 4096;
const int EPOLLWAIT_TIME = 10000;
Epoll::Epoll():
epollFd_(epoll_create(EPOLLCLOEXEC)),//flag
	events_(EVENTNUM)
{
	assert(epollFd_>0);
}
Epoll::~Epoll()
{}



//注册新描述符
void Epoll::epoll_add(SP_Channel request,int timeout)
{
	int fd = request->getFd() ;
	if (timeout > 0)
	{
		add_timer(request,timeout);
		
	}
	struct epoll_event event;
	event.data.fd = fd ;
	event.events = request->getEvents();
	//UpdateLastEvents
	request->isEqualAndUpdateLastEvents();
	fd2chan_[fd] = request ;

	if (epoll_ctl(epollfFd_,EPOLL_CTL_ADD,fd,&event) < 0)
	{
		perror("epoll_add error");
		fd2chan_[fd].reset;
	}
}

//修改描述符状态
void Epoll::epoll_mod(SP_Channel request,int timeout)
{
	if (timeout > 0)
	{
		add_timer(request,timeout);
	}
	int fd = request->getFd();
	if (!request->isEqualAndUpdateLastEvents())
	{
		struct epoll_event event;
		event.data.fd = fd ;
		event.events = request->getEvents();
		if (epoll_ctl(epollFd_,EPOLL_CTL_MOD,fd,&event) < 0)
		{
			perror("epoll_mod error");
			fd2chan_[fd].reset();
		}
	}
}

//从epoll中删除描述符
void Epoll::epoll_del(SP_Channel request)
{
	int fd = request->getFd();
	struct epoll_event event;
	event.data.fd = fd ;
	event.events = request->getEvents();
	if (epoll_ctl(epollFd_,EPOLL_CTL_DEL,fd,&event) < 0)
	{
		perror("epoll_del error");
	}
	fd2chan_[fd].reset();
}

std::vector<SP_Channel> Epoll::poll()
{
	while (true)
	{
		int event_count = epoll_wait(epollFd_,&*events_.begin(),events_.size(),EPOLLWAIT_TIME);
		if(event_count < 0)
			perror("epoll wait error");
		std::vector<SP_Channel> activeChannels = getEventsRequest(event_count);
		if(req_data.size() > 0)
			return activeChannels;
	}
}

//分发处理函数
std::vector<SP_Channel> Epoll::getEventsRequest(int event_count)
{
	std::vector<SP_Channel> activeChannels;
	for (int i = 0;i < event_count;i++)
	{
		//获取有事件产生的描述符
		int fd = events_[i].data.fd;
		SP_Channel cur_activeChannel = fd2chan_[fd] ;
		if (cur_activeChannel)
		{
			cur_activeChannel->setRevents(events_[i].events);
			cur_activeChannel->setEvents(0);//是因为拿到事件了就不期望在这段时间内在监控任何事件
			activeChannels.push_back(cur_activeChannel);
		}
		else
		{
			LOG << "cur_activeChannel is invalid";
		}
	}
	return activeChannels;
}


void Epoll::handleExpired()
{
	timerQueue_.handleExpiredTimer();
}

