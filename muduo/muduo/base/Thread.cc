// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

// 你不得不说，thread确实封装得非常漂亮

#include <muduo/base/Thread.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Exception.h>
#include <muduo/base/Logging.h>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/weak_ptr.hpp>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>

namespace muduo
{
	namespace CurrentThread
	{
		__thread int t_cachedTid = 0;
		__thread char t_tidString[32];
		__thread int t_tidStringLength = 6;
		__thread const char* t_threadName = "unknown"; // 好吧，我没有想到的是，这个玩意居然指向全局区
		const bool sameType = boost::is_same<int, pid_t>::value; // 总之就是判断是否是同一个类型吧！ int以及pid_t
		BOOST_STATIC_ASSERT(sameType);
	}

	namespace detail
	{

		pid_t gettid()
		{
			return static_cast<pid_t>(::syscall(SYS_gettid)); // 得到线程的id号
		}

		void afterFork()
		{
			muduo::CurrentThread::t_cachedTid = 0;
			muduo::CurrentThread::t_threadName = "main";
			CurrentThread::tid(); // 说实话，这一句有作用吗？如果缓存了tid的话，可能还有一些作用,当然，缓存了tid
			// no need to call pthread_atfork(NULL, NULL, &afterFork);
		}

		class ThreadNameInitializer // 用来初始化线程的类吗?
		{
		public:
			ThreadNameInitializer()
			{
				muduo::CurrentThread::t_threadName = "main"; // 当前的线程
				CurrentThread::tid(); 
				pthread_atfork(NULL, NULL, &afterFork); // 这一般表示子线程要执行afterFork函数啊！
			}
		};

		ThreadNameInitializer init; // 一个全局的init

		struct ThreadData // 用于存储线程的一些数据信息
		{
			typedef muduo::Thread::ThreadFunc ThreadFunc;
			ThreadFunc func_; // 比如说，要执行的函数
			string name_; // 线程的名字
			boost::weak_ptr<pid_t> wkTid_; // 一个tid，居然是弱引用

			ThreadData(const ThreadFunc& func,
				const string& name,
				const boost::shared_ptr<pid_t>& tid)
				: func_(func)
				, name_(name)
				, wkTid_(tid)
			{}

			void runInThread()
			{
				pid_t tid = muduo::CurrentThread::tid();

				boost::shared_ptr<pid_t> ptid = wkTid_.lock();
				if (ptid) // 如果线程还活着的话
				{
					*ptid = tid;
					ptid.reset();
				}

				muduo::CurrentThread::t_threadName = name_.empty() ? "muduoThread" : name_.c_str();
				::prctl(PR_SET_NAME, muduo::CurrentThread::t_threadName); // 这个函数用于给线程命名，这样的话，在ps命令中就可以查看到对应的名称
				// 我这里要说一声的是，__thread关键字真是一个好东西。这里的t_threadName这样的变量，每一个线程里面都会有一份的。
				try
				{
					func_(); // 运行对应的函数
					muduo::CurrentThread::t_threadName = "finished"; 
				}
				catch (const Exception& ex)
				{
					muduo::CurrentThread::t_threadName = "crashed"; // 所有的状态信息都存放在t_threadName中
					fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
					fprintf(stderr, "reason: %s\n", ex.what());
					fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
					abort();
				}
				catch (const std::exception& ex)
				{
					muduo::CurrentThread::t_threadName = "crashed";
					fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
					fprintf(stderr, "reason: %s\n", ex.what());
					abort();
				}
				catch (...)
				{
					muduo::CurrentThread::t_threadName = "crashed";
					fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
					throw; // rethrow
				}
			}
		};

		void* startThread(void* obj) // 开始运行线程
		{
			ThreadData* data = static_cast<ThreadData*>(obj); // 首先的话，得到一个ThreadData的数据
			data->runInThread(); // 真的开始运行线程了
			delete data;
			return NULL;
		}

	}
}

using namespace muduo;

void CurrentThread::cacheTid()
{
	if (t_cachedTid == 0)
	{
		t_cachedTid = detail::gettid(); // 得到线程的id号，并缓存起来
		t_tidStringLength = snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid); // 什么string啊，不顾是tid对应的字符形式而已
		// 好吧，在这里还要澄清一下，那就是t_threadName和t_tidString并不是同一个玩意
	}
}

bool CurrentThread::isMainThread() // 用于判断当前线程是否为主线程
{
	return tid() ==:  : getpid(); // 也就是说，主线程的tid和pid是一致的喽！
}

void CurrentThread::sleepUsec(int64_t usec) // 这个是代表休眠多久吧！
{
	struct timespec ts = { 0, 0 };
	ts.tv_sec = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<long>(usec % Timestamp::kMicroSecondsPerSecond * 1000);
	::nanosleep(&ts, NULL);
}

AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc& func, const string& n)
	: started_(false)
	, joined_(false)
	, pthreadId_(0)
	, tid_(new pid_t(0))
	, func_(func)
	, name_(n)
{
	setDefaultName();
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__ // 说明这些个玩意是实验性质的，是吧！
Thread::Thread(ThreadFunc&& func, const string& n)
	: started_(false)
	, joined_(false)
	, pthreadId_(0)
	, tid_(new pid_t(0))
	, func_(std::move(func))
	, name_(n) // 初始化的函数
{
	setDefaultName();
}

#endif

Thread::~Thread() // 析构函数
{
	if (started_ && !joined_)
	{
		pthread_detach(pthreadId_);
	}
}

void Thread::setDefaultName()
{
	int num = numCreated_.incrementAndGet(); // numCreated是一个static类型，表示线程的计数啦！
	if (name_.empty())
	{
		char buf[32];
		snprintf(buf, sizeof buf, "Thread%d", num);
		name_ = buf;
	}
}

void Thread::start()
{
	assert(!started_);
	started_ = true;
	// FIXME: move(func_)
	detail::ThreadData* data = new detail::ThreadData(func_, name_, tid_);
	if (pthread_create(&pthreadId_, NULL, &detail::startThread, data))
	{
		started_ = false;
		delete data; // or no delete?
		LOG_SYSFATAL << "Failed in pthread_create";
	}
}

int Thread::join()
{
	assert(started_);
	assert(!joined_);
	joined_ = true;
	return pthread_join(pthreadId_, NULL);
}

