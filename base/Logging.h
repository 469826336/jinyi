// @file    Logging.h
// @author  axunzzz
// @date    2019/11/20
// @Email   469826336@qq.com
#pragma once
#include <pthread.h>
#include <stdio.h>
// <string.h>�Ǿɵ�C ͷ�ļ�����Ӧ���ǻ���char*���ַ�����������
// <string>�ǰ�װ��std ��C++ͷ�ļ�����Ӧ�����µ�string ��
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