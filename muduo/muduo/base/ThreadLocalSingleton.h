// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_THREADLOCALSINGLETON_H
#define MUDUO_BASE_THREADLOCALSINGLETON_H

#include <boost/noncopyable.hpp>
#include <assert.h>
#include <pthread.h>

namespace muduo
{

	template<typename T>
		class ThreadLocalSingleton : boost::noncopyable
		{
		public:

			static T& instance() // 构建一个实例
			{
				if (!t_value_) // 如果还没有构建一个呢
				{
					t_value_ = new T(); // 好吧，构建一个
					deleter_.set(t_value_);
				}
				return *t_value_;
			}

			static T* pointer()
			{
				return t_value_;
			}

		private:
			ThreadLocalSingleton(); // 这里就显得意味深长了，也就是不允许复制
			~ThreadLocalSingleton();

			static void destructor(void* obj)
			{
				assert(obj == t_value_);
				typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
				T_must_be_complete_type dummy; (void) dummy;
				delete t_value_;
				t_value_ = 0;
			}

			class Deleter
			{
			public:
				Deleter()
				{
					pthread_key_create(&pkey_, &ThreadLocalSingleton::destructor); // 好吧，初始化一个key
				}

				~Deleter()
				{
					pthread_key_delete(pkey_);
				}

				void set(T* newObj)
				{
					assert(pthread_getspecific(pkey_) == NULL);
					pthread_setspecific(pkey_, newObj);
				}

				pthread_key_t pkey_;
			};

			static __thread T* t_value_;
			static Deleter deleter_; // 果然是单件模式
		};

	template<typename T>
		__thread T* ThreadLocalSingleton<T>::t_value_ = 0;

	template<typename T>
		typename ThreadLocalSingleton<T>::Deleter ThreadLocalSingleton<T>::deleter_;

}
#endif
