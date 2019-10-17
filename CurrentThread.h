#pragma once
//<stdint.h>������int16_t,uint32_t,int64_t�����ͣ�������ʹ��64λ����
#include <stdint.h>



namespace CurrentThread
{
	//internal,__thread��GCC���õ��ֲ߳̾�������ʩ��__thread����ÿһ���߳���һ�ݶ���ʵ��
	extern __thread int t_cacheTid;
	extern __thread char t_tidString[32];
	extern __thread int t_tidStringLength;
	extern __thread const char *t_threadName;
	void cacheTid();
	inline int tid()
	{
		//__builtin_expect��Ҫ�������ڻ����Ż����ǵĴ��룬������ת�Ĵ���
        //https://blog.csdn.net/huntinux/article/details/51995913
		if (__builtin_expect(t_cachedTid==0),0)
		{
			cacheTid();
		}
		return t_cacheTid;
	}






}