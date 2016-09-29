#ifndef MUDUO_BASE_LOGSTREAM_H
#define MUDUO_BASE_LOGSTREAM_H

#include <muduo/base/StringPiece.h>
#include <muduo/base/Types.h>
#include <assert.h>
#include <string.h> // memcpy
#ifndef MUDUO_STD_STRING
#include <string>
#endif
#include <boost/noncopyable.hpp>

// 我很好奇的是Buffer究竟是哪一种数据类型
namespace muduo
{

	namespace detail // muduo::detail
	{

		const int kSmallBuffer = 4000; // 好吧，非常漂亮
		const int kLargeBuffer = 4000 * 1000;

		template<int SIZE>
		class FixedBuffer : boost::noncopyable
		{
		public:
			FixedBuffer()
				: cur_(data_)
			{
				setCookie(cookieStart);
			}

			~FixedBuffer()
			{
				setCookie(cookieEnd);
			}

			void append(const char* /*restrict*/ buf, size_t len)
			{
				// FIXME: append partially
				if (implicit_cast<size_t>(avail()) > len)
				{
					memcpy(cur_, buf, len); // 内存复制
					cur_ += len; // cur_指的是当前的指针
				}
			}

			const char* data() const { return data_; } // 直接将内部的数据结构抛了出去
			int length() const { return static_cast<int>(cur_ - data_); }

			// write to data_ directly
			char* current() { return cur_; }
			int avail() const { return static_cast<int>(end() - cur_); } // avail返回当前的余量
			void add(size_t len) { cur_ += len; } // 指针直接往后移动

			void reset() { cur_ = data_; } // 重新设置
			void bzero() { ::bzero(data_, sizeof data_); } // 清零

			// for used by GDB
			const char* debugString();
			void setCookie(void(*cookie)()) { cookie_ = cookie; } // 设置回调函数吧，应该
			// for used by unit test
			string toString() const { return string(data_, length()); } // 
			StringPiece toStringPiece() const { return StringPiece(data_, length()); }

		private:
			const char* end() const { return data_ + sizeof data_; }
			// Must be outline function for cookies.
			static void cookieStart();
			static void cookieEnd();

			void(*cookie_)(); // cookie是一个函数指针
			char data_[SIZE]; // 终于看见实际的玩意了
			char* cur_; // 这个指针指向data数组，表示当前已经输入字符串的位置
		};

	}

	class LogStream : boost::noncopyable
	{
		typedef LogStream self;
	public:
		typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer; // 好吧，看见了Buffer的定义了！

		self& operator<<(bool v) // 好吧，返回的果然是LogStream类型
		{
			buffer_.append(v ? "1" : "0", 1); // 书写bool类型
			return *this;
		}

		self& operator<<(short);
		self& operator<<(unsigned short);
		self& operator<<(int);
		self& operator<<(unsigned int);
		self& operator<<(long);
		self& operator<<(unsigned long);
		self& operator<<(long long);
		self& operator<<(unsigned long long);

		self& operator<<(const void*);

		self& operator<<(float v)
		{
			*this << static_cast<double>(v);
			return *this;
		}
		self& operator<<(double);
		// self& operator<<(long double);

		self& operator<<(char v)
		{
			buffer_.append(&v, 1);
			return *this;
		}

		// self& operator<<(signed char);
		// self& operator<<(unsigned char);

		self& operator<<(const char* str)
		{
			if (str) // 如果不为空
			{
				buffer_.append(str, strlen(str));
			}
			else
			{
				buffer_.append("(null)", 6); // 居然添加了一个null
			}
			return *this;
		}

		self& operator<<(const unsigned char* str)
		{
			return operator<<(reinterpret_cast<const char*>(str));
		}

		self& operator<<(const string& v)
		{
			buffer_.append(v.c_str(), v.size());
			return *this;
		}

#ifndef MUDUO_STD_STRING
		self& operator<<(const std::string& v)
		{
			buffer_.append(v.c_str(), v.size());
			return *this;
		}
#endif

		self& operator<<(const StringPiece& v)
		{
			buffer_.append(v.data(), v.size());
			return *this;
		}

		self& operator<<(const Buffer& v)
		{
			*this << v.toStringPiece();
			return *this;
		}

		void append(const char* data, int len) { buffer_.append(data, len); }
		const Buffer& buffer() const { return buffer_; }
		void resetBuffer() { buffer_.reset(); }

	private:
		void staticCheck();

		template<typename T>
		void formatInteger(T);

		Buffer buffer_; // buffer_是一个好东西

		static const int kMaxNumericSize = 32;
	};

	class Fmt // : boost::noncopyable
	{ // 这个玩意表示的是格式吧!
	public:
		template<typename T>
		Fmt(const char* fmt, T val); // 所谓的格式，是char* 类型的fmt，以及对应的值val

		const char* data() const { return buf_; }
		int length() const { return length_; }

	private:
		char buf_[32];
		int length_;
	};

	inline LogStream& operator<<(LogStream& s, const Fmt& fmt)
	{
		s.append(fmt.data(), fmt.length());
		return s;
	}

}
#endif  // MUDUO_BASE_LOGSTREAM_H

