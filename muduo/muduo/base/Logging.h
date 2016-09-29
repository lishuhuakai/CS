#ifndef MUDUO_BASE_LOGGING_H
#define MUDUO_BASE_LOGGING_H

#include <muduo/base/LogStream.h>
#include <muduo/base/Timestamp.h>

namespace muduo
{

	class TimeZone; // 好吧，终于扯到时区了

	class Logger
	{
	public:
		enum LogLevel // 枚举类型
		{
			TRACE,
			DEBUG,
			INFO,
			WARN,
			ERROR,
			FATAL,
			NUM_LOG_LEVELS,
		};

		  // compile time calculation of basename of source file
		class SourceFile // 好吧，第一次看见，类里面可以嵌套类
		{
		public:
			template<int N>
				inline SourceFile(const char(&arr)[N])
					: data_(arr)
					, size_(N - 1)
				{
					const char* slash = strrchr(data_, '/'); // 找到最后一个出现的'/'
					if (slash) // 如果存在slash
					{
						data_ = slash + 1;
						size_ -= static_cast<int>(data_ - arr);
					}
				}

			explicit SourceFile(const char* filename)
				: data_(filename)
			{
				const char* slash = strrchr(filename, '/');
				if (slash)
				{
					data_ = slash + 1;
				}
				size_ = static_cast<int>(strlen(data_));
			}

			const char* data_;
			int size_;
		};

		Logger(SourceFile file, int line); // 好一个记录的玩意
		Logger(SourceFile file, int line, LogLevel level);
		Logger(SourceFile file, int line, LogLevel level, const char* func);
		Logger(SourceFile file, int line, bool toAbort);
		~Logger();

		LogStream& stream() { return impl_.stream_; }

		static LogLevel logLevel(); // 这里应该有一个control level吧！
		static void setLogLevel(LogLevel level);

		typedef void(*OutputFunc)(const char* msg, int len);
		typedef void(*FlushFunc)();
		static void setOutput(OutputFunc);
		static void setFlush(FlushFunc); // 好吧，居然还有setFlush函数
		static void setTimeZone(const TimeZone& tz);

	private:

		class Impl // 内嵌类
		{
		public:
			typedef Logger::LogLevel LogLevel;
			Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
			void formatTime();
			void finish();

			Timestamp time_; // 时戳
			LogStream stream_; // 一个流对象
			LogLevel level_;
			int line_;
			SourceFile basename_;
		};

		Impl impl_; // 一个实现吧！

	};

	extern Logger::LogLevel g_logLevel; // 一个全局的变量

	inline Logger::LogLevel Logger::logLevel()
	{
		return g_logLevel; // 记录的层次
	}

	//
	// CAUTION: do not write:
	//
	// if (good)
	//   LOG_INFO << "Good news";
	// else
	//   LOG_WARN << "Bad news";
	//
	// this expends to
	//
	// if (good)
	//   if (logging_INFO)
	//     logInfoStream << "Good news";
	//   else
	//     logWarnStream << "Bad news";
	//
#define LOG_TRACE if (muduo::Logger::logLevel() <= muduo::Logger::TRACE) \
	  muduo::Logger(__FILE__, __LINE__, muduo::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG) \
	    muduo::Logger(__FILE__, __LINE__, muduo::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (muduo::Logger::logLevel() <= muduo::Logger::INFO) \
	      muduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN muduo::Logger(__FILE__, __LINE__, muduo::Logger::WARN).stream()
#define LOG_ERROR muduo::Logger(__FILE__, __LINE__, muduo::Logger::ERROR).stream()
#define LOG_FATAL muduo::Logger(__FILE__, __LINE__, muduo::Logger::FATAL).stream()
#define LOG_SYSERR muduo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL muduo::Logger(__FILE__, __LINE__, true).stream()

	const char* strerror_tl(int savedErrno);

	// Taken from glog/logging.h
	//
	// Check that the input is non NULL.  This very useful in constructor
	// initializer lists.

#define CHECK_NOTNULL(val) \
	  ::muduo::CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

	  // A small helper for CHECK_NOTNULL().
	template <typename T>
		T* CheckNotNull(Logger::SourceFile file, int line, const char *names, T* ptr)
		{
			if (ptr == NULL)
			{
				Logger(file, line, Logger::FATAL).stream() << names; // fatal的话，表示致命的错误吧！
			}
			return ptr;
		}

}

#endif  // MUDUO_BASE_LOGGING_H
