// @file    noncopyable.h
// @author  axunzzz
// @date    2019/10/22
// @Email   469826336@qq.com
#pragma once

//设计成不可拷贝的好处是 这样就强制只能以指针的方式使用； 就不能以拷贝类的方式来使用他啦， 因为反复拷贝的话要不断的创建内存空间，用指针的话开销要小的多
class noncopyable
{
protected:
	noncopyable(){}
	~noncopyable(){}

private:
	noncopyable(const noncopyable&);
	const noncopyable& operator=(const noncopyable&);
};
