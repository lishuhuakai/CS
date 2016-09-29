// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_ATOMIC_H
#define MUDUO_BASE_ATOMIC_H

#include <boost/noncopyable.hpp>
#include <stdint.h>

namespace muduo
{

	namespace detail
	{
		template<typename T>
			class AtomicIntegerT : boost::noncopyable
			{
			public:
				AtomicIntegerT()
					: value_(0) // 首先value_赋值为0
				{
				}

				  // uncomment if you need copying and assignment
				  //
				  // AtomicIntegerT(const AtomicIntegerT& that)
				  //   : value_(that.get())
				  // {}
				  //
				  // AtomicIntegerT& operator=(const AtomicIntegerT& that)
				  // {
				  //   getAndSet(that.get());
				  //   return *this;
				  // }

				T get()
				{
				  // in gcc >= 4.7: __atomic_load_n(&value_, __ATOMIC_SEQ_CST)
					// 下面的函数是gcc的原子操作类型
					return __sync_val_compare_and_swap(&value_, 0, 0);
					// 代码等价于
					//if (value_ == 0)
					//{
					//	value_ = 0;
					//}
					//return 0;
				}

				T getAndAdd(T x)
				{
				  // in gcc >= 4.7: __atomic_fetch_add(&value_, x, __ATOMIC_SEQ_CST)
					// 好吧，又是一个原子操作其实就是相当于value_ = value_ + x;
					return __sync_fetch_and_add(&value_, x);
				}

				T addAndGet(T x)
				{
					return getAndAdd(x) + x;
				}

				T incrementAndGet()
				{
					// 增加1
					return addAndGet(1);
				}

				T decrementAndGet()
				{
					// 减去1
					return addAndGet(-1);
				}

				void add(T x)
				{
					// 增加x
					getAndAdd(x);
				}

				void increment()
				{
					incrementAndGet();
				}

				void decrement()
				{
					decrementAndGet();
				}

				T getAndSet(T newValue)
				{
				  // in gcc >= 4.7: __atomic_store_n(&value, newValue, __ATOMIC_SEQ_CST)
					// 总之就是给value_赋值为新值，然后返回老值
					return __sync_lock_test_and_set(&value_, newValue);
				}

			private:
				volatile T value_;
			};
	}

	typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
	typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
}

#endif  // MUDUO_BASE_ATOMIC_H
