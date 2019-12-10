// @file    LogStream.cpp
// @author  axunzzz
// @date    2019/11/21
// @Email   469826336@qq.com
#include "LogStream.h"
#include <assert.h>
#include <stdint.h>
#include<stdint.h>
#include <stdio.h>//snprintf()
#include<string.h>
#include <algorithm>
// #include <limits>

const char digits[] = "9876543210123456789";
const char *zero = digits + 9;

// From muduo
template <typedname T>
size_t convert(char buf[],T value)
{
	T i = value;
	char *p = buf;

	do 
	{
		int lsd = static_cast<int>(i % 10);
		i /= 10;
		*p++ = zero[lsd];
	} while (i != 0);

	if (value < 0)
	{
		*p++ = '-';
	}
	*p = '\0';
	std::reverse(buf,p);

	return p - buf;
}

// 模板特例化
template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;


template <typename T>
void LogStream::formateInteger(T v)
{
	// buffer容不下kMaxNumericSize个字符的话会被直接丢弃
	if (buffer_.avail() >= kMaxNumericSize)
	{
		size_t len = convert(buffer_.current(),v);
		buffer_.add(len);
	}
}

LogStream& LogStream::operator <<(short v)
{
	*this << static_cast<int>(v);
	return *this;
}

LogStream& LogStream::operator <<(unsigned short v)
{
	*this << static_cast<int>(v);
	return *this;
}

LogStream& LogStream::operator<<(int v)
{
	formateInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
	formateInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(long v)
{
	formateInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
	formateInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(long long v)
{
	formateInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
	formateInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(float v)
{
	*this<<static_cast<double>(v);
	return *this;
}

LogStream& LogStream::operator<<(double v)
{
	if (buffer_.avail() > kMaxNumericSize)
	{
		int len = snprintf(buffer_.current(),kMaxNumericSize,"%.12g",v);
		buffer_.add(len);
	}
	return *this;
}

LogStream& LogStream::operator<<(long double v)
{
	if (buffer_.avail() > kMaxNumericSize)
	{
		int len = snprintf(buffer_.current(),kMaxNumericSize,"%.12g",v);
		buffer_.add(len);
	}
	return *this;
}