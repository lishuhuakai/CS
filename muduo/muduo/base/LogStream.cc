#include <muduo/base/LogStream.h>

#include <algorithm>
#include <limits>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

using namespace muduo;
using namespace muduo::detail;

#if defined(__clang__)
#pragma clang diagnostic ignored "-Wtautological-compare"
#else
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

namespace muduo
{
	namespace detail
	{

		const char digits[] = "9876543210123456789";// 对应的数字
		const char* zero = digits + 9; // 这个玩意恰好指向0
		BOOST_STATIC_ASSERT(sizeof(digits) == 20);

		const char digitsHex[] = "0123456789ABCDEF"; // 16进制的数
		BOOST_STATIC_ASSERT(sizeof digitsHex == 17);

		// Efficient Integer to String Conversions, by Matthew Wilson.
		template<typename T>
			size_t convert(char buf[], T value) // 转换
			{
				T i = value;
				char* p = buf;

				do
				{
					int lsd = static_cast<int>(i % 10);
					i /= 10;
					*p++ = zero[lsd]; // 得到对应的数字
				} while (i != 0);

				if (value < 0) // 我唯一有一点不明白的地方就是，为什么这个减号要加在后面
				{
					*p++ = '-';
				}
				*p = '\0';
				std::reverse(buf, p); // 好吧，我想，我懂了，这里出现了反转

				return p - buf; // 返回长度
			}

		size_t convertHex(char buf[], uintptr_t value) // 将hex类型转换成为char类型吧
		{
			uintptr_t i = value; // 得到对应的值
			char* p = buf;

			do
			{
				int lsd = static_cast<int>(i % 16);
				i /= 16;
				*p++ = digitsHex[lsd];
			} while (i != 0);

			*p = '\0';
			std::reverse(buf, p);

			return p - buf;
		}

		template class FixedBuffer<kSmallBuffer>;
		template class FixedBuffer<kLargeBuffer>;

	}
}

template<int SIZE>
	const char* FixedBuffer<SIZE>::debugString()
	{
		*cur_ = '\0';
		return data_;
	}

template<int SIZE>
	void FixedBuffer<SIZE>::cookieStart()
	{
	}

template<int SIZE>
	void FixedBuffer<SIZE>::cookieEnd()
	{ // 好吧，这些个玩意里面什么也不干
	}

void LogStream::staticCheck()
{
	BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10);
	BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10);
	BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10);
	BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10);
}

template<typename T>
	void LogStream::formatInteger(T v) // 用于格式化字符串
	{
		if (buffer_.avail() >= kMaxNumericSize) // 有足够的余量
		{
			size_t len = convert(buffer_.current(), v);
			buffer_.add(len);
		}
	}

LogStream& LogStream::operator<<(short v)
{
	*this << static_cast<int>(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned short v)
{
	*this << static_cast<unsigned int>(v);
	return *this;
}

LogStream& LogStream::operator<<(int v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned int v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(long v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(long long v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(unsigned long long v)
{
	formatInteger(v);
	return *this;
}

LogStream& LogStream::operator<<(const void* p)
{
	uintptr_t v = reinterpret_cast<uintptr_t>(p);// 好吧，转得非常漂亮
	if (buffer_.avail() >= kMaxNumericSize)
	{
		char* buf = buffer_.current();
		buf[0] = '0';
		buf[1] = 'x'; // 表示以0x开头的字符
		size_t len = convertHex(buf + 2, v);
		buffer_.add(len + 2); // 好吧，输入了对应的16进制
	}
	return *this;
}

// FIXME: replace this with Grisu3 by Florian Loitsch.
LogStream& LogStream::operator<<(double v)
{
	if (buffer_.avail() >= kMaxNumericSize)
	{
		int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v); // 好吧，终于看到了使用snprintf了，一阵舒爽啊！
		buffer_.add(len);
	}
	return *this;
}

template<typename T>
	Fmt::Fmt(const char* fmt, T val) // 终于看到了所谓的fmt了
	{
		BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value == true);

		length_ = snprintf(buf_, sizeof buf_, fmt, val); // 好吧，这些玩意差不多都是由c语言的一些库函数来完成的是吧.
		assert(static_cast<size_t>(length_) < sizeof buf_);
	}

	// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);
