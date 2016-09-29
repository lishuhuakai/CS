#include <muduo/base/BlockingQueue.h>
#include <muduo/base/CountDownLatch.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Timestamp.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <map>
#include <string>
#include <stdio.h>

class Bench
{
public:
	Bench(int numThreads) // 线程的数目
		: latch_(numThreads) 
		, threads_(numThreads)
	{
		for (int i = 0; i < numThreads; ++i)
		{
			char name[32];
			snprintf(name, sizeof name, "work thread %d", i);
			threads_.push_back(new muduo::Thread(
			      boost::bind(&Bench::threadFunc, this), // 这个是要运行的函数
				muduo::string(name))); // 就是构建线程，然后往threads_里面放
		}
		for_each(threads_.begin(), threads_.end(), boost::bind(&muduo::Thread::start, _1)); // 这里有一些函数式编程的味道了
		// 这里的话，我要说一下了boost::bind(&moduo::Thread::start, -1)构建了一个函数，记为t，然后将调用t(arg)
	}

	void run(int times)
	{
		printf("waiting for count down latch\n");
		latch_.wait(); // 等待信号吧！
		printf("all threads started\n"); // 所有的线程都已经开始运行了。
		for (int i = 0; i < times; ++i)
		{
			muduo::Timestamp now(muduo::Timestamp::now());
			queue_.put(now); // 我有点不懂的是，为什么要往里面放入这么多的Timestamp
			usleep(1000);
		}
	}

	void joinAll()
	{
		for (size_t i = 0; i < threads_.size(); ++i)
		{
			queue_.put(muduo::Timestamp::invalid());
		}

		for_each(threads_.begin(), threads_.end(), boost::bind(&muduo::Thread::join, _1)); // 对于每一个进程都调用join函数
	}

private:

	void threadFunc()
	{
		printf("tid=%d, %s started\n",
			muduo::CurrentThread::tid(),  // 线程的id号
			muduo::CurrentThread::name()); // 线程的名字

		std::map<int, int> delays;
		latch_.countDown(); // 倒计时一样的东西
		bool running = true;
		while (running)
		{
			muduo::Timestamp t(queue_.take()); // 不断地取时间戳
			muduo::Timestamp now(muduo::Timestamp::now());
			if (t.valid()) // 
			{
				int delay = static_cast<int>(timeDifference(now, t) * 1000000);
				// printf("tid=%d, latency = %d us\n",
				//        muduo::CurrentThread::tid(), delay);
				++delays[delay]; // 延时
			}
			running = t.valid();
		}

		printf("tid=%d, %s stopped\n",
			muduo::CurrentThread::tid(),
			muduo::CurrentThread::name()); // 表示线程停止了吗？
		for (std::map<int, int>::iterator it = delays.begin();
		    it != delays.end(); ++it)
		{
			printf("tid = %d, delay = %d, count = %d\n",
				muduo::CurrentThread::tid(),
				it->first,  // key
				it->second); // value
		}
	}

	muduo::BlockingQueue<muduo::Timestamp> queue_; // 时戳
	muduo::CountDownLatch latch_; // 好吧，居然是这个玩意
	boost::ptr_vector<muduo::Thread> threads_; // 指针的vector，还好，还好
};

int main(int argc, char* argv[])
{
	int threads = argc > 1 ? atoi(argv[1]) : 1;
	threads = 120;
	Bench t(threads);
	t.run(10000); // 运行1000个线程吗？
	t.joinAll();
}
