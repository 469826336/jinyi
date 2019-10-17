#pragma once
//<stdint.h>定义了int16_t,uint32_t,int64_t等类型，大整数使用64位整型
#include <stdint.h>



namespace CurrentThread
{
	//internal,__thread是GCC内置的线程局部储存设施，__thread变量每一个线程有一份独立实体
	extern __thread int t_cacheTid;
	extern __thread char t_tidString[32];
	extern __thread int t_tidStringLength;
	extern __thread const char *t_threadName;
	void cacheTid();
	inline int tid()
	{
		//__builtin_expect主要作用是在汇编层优化我们的代码，减少跳转的次数
        //https://blog.csdn.net/huntinux/article/details/51995913
		if (__builtin_expect(t_cachedTid==0),0)
		{
			cacheTid();
		}
		return t_cacheTid;
	}






}