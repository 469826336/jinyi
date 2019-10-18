#include "Epoll.h"
#include <error.h>
//socket.h�����õ�
//#include <sys/socket.h>
//������tcp/ip�йص�ͷ�ļ�
//#include <netinet/in.h>
//#include <arpa/inet.h>
//Ҳû����queue��deque
#include<assert.h>
//Ҳû����iostream


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



//ע����������
void Epoll::epoll_add(SP_Channel request,int timeout)
{
	int fd = request->getFd() ;
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

//�޸�������״̬
void Epoll::epoll_add(SP_Channel request,int timeout)
{
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

//��epoll��ɾ��������
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

std::vector<SP_Channel> Epoll::Epoll()
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

//�ַ�������
std::vector<SP_Channel> Epoll::getEventsRequest(int event_count)
{
	std::vector<SP_Channel> activeChannels;
	for (int i = 0;i < event_count;i++)
	{
		//��ȡ���¼�������������
		int fd = events_[i].data.fd;
		SP_Channel cur_activeChannel = fd2chan_[fd] ;
		if (cur_activeChannel)
		{
			cur_activeChannel->setRevents(events_[i].events);
			cur_activeChannel->setEvents(0);//����Ϊ�õ��¼��˾Ͳ����������ʱ�����ڼ���κ��¼�
			activeChannels.push_back(cur_activeChannel);
		}
		else
		{
			LOG << "cur_activeChannel is invalid";
		}
	}
	return activeChannels;
}


