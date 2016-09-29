#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Timestamp.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <stdio.h>

using namespace muduo;
using namespace std;

MutexLock g_mutex; // 一个全局的锁
vector<int> g_vec; // 一个全局的vector
const int kCount = 10 * 1000 * 1000;

void threadFunc()
{
	for (int i = 0; i < kCount; ++i)
	{
		MutexLockGuard lock(g_mutex); // 首先加锁是吧，然后往全局变量g_vec里面放入
		g_vec.push_back(i);
	}
}

int foo() __attribute__((noinline));

int g_count = 0; // 一个全局的g_count
int foo()
{
	MutexLockGuard lock(g_mutex); // 加锁
	if (!g_mutex.isLockedByThisThread()) // 如果这个锁不是被这个线程给占有的话
	{
		printf("FAIL\n");
		return -1;
	}

	++g_count;
	return 0;
}

int main()
{
	MCHECK(foo());
	if (g_count != 1)
	{
		printf("MCHECK calls twice.\n");
		abort();
	}

	const int kMaxThreads = 8;
	g_vec.reserve(kMaxThreads * kCount); // 首先保留这么多的空间

	Timestamp start(Timestamp::now());
	for (int i = 0; i < kCount; ++i)
	{
		g_vec.push_back(i);
	}

	printf("single thread without lock %f\n", timeDifference(Timestamp::now(), start));

	start = Timestamp::now();
	threadFunc();
	printf("single thread with lock %f\n", timeDifference(Timestamp::now(), start));
	// 单进程加了锁之后，耗时增加得非常快

	for (int nthreads = 1; nthreads < kMaxThreads; ++nthreads)
	{
		boost::ptr_vector<Thread> threads;
		g_vec.clear();
		start = Timestamp::now();
		for (int i = 0; i < nthreads; ++i)
		{
			threads.push_back(new Thread(&threadFunc));
			threads.back().start();
		}
		for (int i = 0; i < nthreads; ++i)
		{
			threads[i].join();
		}
		printf("%d thread(s) with lock %f\n", nthreads, timeDifference(Timestamp::now(), start));
	}
}

