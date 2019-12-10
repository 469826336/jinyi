// @file    FileUtil.h
// @author  axunzzz
// @date    2019/11/19
// @Email   469826336@qq.com
#pragma once
#include<string>
#include "noncopyable.h"

class AppendFile : noncopyable
{
public:
	explicit AppendFile(std::string filename);
	~AppendFile();
	// append�����ļ�д
	void append(const char *logline,const size_t len);
	void flush();

private:
	size_t write(const char *logline,size_t len);
	FILE *fp_; // fopen()���ص�ָ�������ļ�ָ��
	char buffer_[64*1024];
};