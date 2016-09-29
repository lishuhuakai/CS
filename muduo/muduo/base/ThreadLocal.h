// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADLOCAL_H
#define MUDUO_BASE_THREADLOCAL_H

#include <muduo/base/Mutex.h>  // MCHECK

#include <boost/noncopyable.hpp>
#include <pthread.h>


namespace muduo
{

	template<typename T>
		class ThreadLocal : boost::noncopyable
		{
		public:
			ThreadLocal()
			{
				// pkey_表示什么玩意？ 好吧，清理函数是Threadlocal::destructor
				// threadLocal我感觉非常类似于gcc的__thread一类的东西
				MCHECK(pthread_key_create(&pkey_, &ThreadLocal::destructor));
			}

			~ThreadLocal()
			{
				MCHECK(pthread_key_delete(pkey_)); // 销毁key指向的玩意
			}

			T& value()
			{
				T* perThreadValue = static_cast<T*>(pthread_getspecific(pkey_)); // 首先得到T*类型的指针，是吧，当然，这个玩意不一定有
				// pthread_getspecific函数用于得到pthread_key_t变量，该变量返回void*类型的值
				
				if (!perThreadValue) // 如果不存在的话
				{
					T* newObj = new T(); // 构建一个新的对象
					MCHECK(pthread_setspecific(pkey_, newObj)); // 传递给pkey_
					perThreadValue = newObj;
				}
				return *perThreadValue; // 每个线程的值
			}

		private:

			static void destructor(void *x) // 析构函数
			{
				T* obj = static_cast<T*>(x); // 因为本来就是T类型的指针嘛，所以转换一下，问题也不是很大。
				typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
				T_must_be_complete_type dummy; (void) dummy;
				delete obj; // 总之就是删除就是啦！
			}

		private:
			pthread_key_t pkey_;
		};

}
#endif
