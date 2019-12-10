// @file    Logging.cpp
// @author  axunzzz
// @date    2019/11/20
// @Email   469826336@qq.com
#include "Logging.h"
// #include "CurrentThread.h"
// #include "Thread.h"
#include "AsyncLogging.h"
#include <assert.h>
// #include<iostream>
#include<time.h>
#include<sys/time.h>
static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_;

std::string Logger::logFileName_ = "./WebServer.log";

void once_init()
{
	AsyncLogging_ = new AsyncLogging(Logger::getLogFileName());
	AsyncLogger_->start();
}

void output(const char *msg,int len)
{
	pthread_once(&once_control_,once_init);
	AsyncLogger_->append(msg,len);
}


// from muduo
// helper class for know tid and tid length at compile time
class T
{
public:
	T(const char *str,unsigned len)
		:str_(str),
		len_(len)
	{
		assert(strlen(str) == len_);
	}

	const char *str_;
	const unsigned len_;
};

inline LogStream& operator<<(LogStream &s,T v)
{
	s.append(v.str_,v.len_);
	return s;
}
Logger::Impl::Impl(const char *fileName,int line)
	:stream_(),
	line_(line),
	basename_(fileName)
{
	formatTime();
	CurrentThread::tid();
	stream_<<T(CurrentThread::tidString(),CurrentThread::tidStringLength());
}

void Logger::Impl::formatTime()
{
	struct timeval tv;
	time_t time;
	char str_t[26] = {0};
	gettimeofday(&tv,NULL);
	time = tv.tv_sec;
	struct tm *p_time = localtime(&time);
	strftime(str_t,26,"%Y-%m-%d %H:%M:%S\n",p_time);
	stream_<<str_t;
}

Logger::Logger(const char *fileName,int line)
	:impl_(fileName,line)
{ }  


Logger::~Logger()
{
	impl_.stream_<<" --"<<impl_.basename_<<':'<<impl_.line_<<'\n';
	const LogStream::Buffer &buf(stream().buffer());
	output(buf.data(),buf.length());
}





