#include "Channel.h"
#include"EventLoop.h"


Channel::Channel(EventLoop *loop):loop_(loop),events_(0)
{}

Channel::Channel(EventLoop *loop,int fd):loop_(loop),fd_(fd),events_(0)
{}

Channel::~Channel()
{
	//因为用到smart_ptr,<memory>?
	//loop_->poller_->epoll_del(fd.events_);
	//close(fd_);
}

int Channel::getFd()
{
	return fd_;
}

void Channel::setFd(int fd)
{
	fd_ = fd ;
}






