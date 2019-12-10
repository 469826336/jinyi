// @file    Logging.h
// @author  axunzzz
// @date    2019/11/20
// @Email   469826336@qq.com
#pragma once
#include <pthread.h>
#include <stdio.h>
// <string.h>是旧的C 头文件，对应的是基于char*的字符串处理函数；
// <string>是包装了std 的C++头文件，对应的是新的string 类
#include<string>
#include<string.h>
#include "LogStream.h"

class AsyncLogging;

class Logger
{
public:
	Logger(const char *fileName,int line);
	~Logger();
	
	LogSteam& stream()
	{
		return impl_.stream_;
	}

	static void setLogFileName(std::string fileName)
	{
		logFileName_ = fileName;
	}

	static std::string getLogFileName()
	{
		return logFileName_;
	}

private:
	class Impl
	{
	public:
		Impl(const char *fileName,int line);
		void formatTime();

		LogStream stream_;
		int line_;
		std::string basename_;
	};
	Impl impl_;
	static std::string logFileName_;
};

#define  LOG Logger(__FILE__,__LINE__).stream();