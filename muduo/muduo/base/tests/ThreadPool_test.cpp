#include <muduo/base/ThreadPool.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/CurrentThread.h>
//#include <muduo/base/Logging.h>
#include <iostream>
#include <boost/bind.hpp>
#include <stdio.h>

void print()
{
	printf("tid=%d\n", muduo::CurrentThread::tid()); // 输出
}

void printString(const std::string& str)
{
	//LOG_INFO << str;
	std::cout << str << std::endl;
	usleep(100 * 1000);
}

void test(int maxSize)
{
	//LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
	std::cout << "Test ThreadPool with max queue size = " << maxSize << std::endl;
	muduo::ThreadPool pool("MainThreadPool");
	pool.setMaxQueueSize(maxSize);
	pool.start(5); // 开启5个线程吗？

	//LOG_WARN << "Adding";
	std::cout << "Adding" << std::endl;
	pool.run(print); // 这就相当于往池子里面添加任务
	pool.run(print);
	for (int i = 0; i < 100; ++i)
	{
		char buf[32];
		snprintf(buf, sizeof buf, "task %d", i);
		pool.run(boost::bind(printString, std::string(buf)));
	}
	//LOG_WARN << "Done";
	std::cout << "Done" << std::endl;

	muduo::CountDownLatch latch(1);
	pool.run(boost::bind(&muduo::CountDownLatch::countDown, &latch));
	latch.wait();
	pool.stop();
}

int main()
{
	test(0); // 对容量无限制
	test(1);
	test(5);
	test(10);
	test(50);
}
