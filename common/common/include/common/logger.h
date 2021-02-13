#pragma once
#include <common/common.h>

#include <sstream>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <atomic>
#include <common/ScopedEnum.h>
#include <common/StringFormat.h>
#include <time.h>
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#pragma warning(disable:4996)
#define BOOST_CHRONO_VERSION 2
#include <boost/chrono.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <iostream>


namespace logger {

	struct Context
	{
		const char *filename;
		int line;
		const char *function;
		Context() :
			line(0),
			filename(""),
			function("")
		{
		}

		Context(const Context& other)
		{
			*this = other;
		}

		Context& operator = (const Context& other)
		{
			filename = other.filename;
			line = other.line;
			function = other.function;
			return *this;
		}
	};
	SCOPED_ENUM(Level,
		None = 0,
		Critical = 1,
		Error = 2,
		Warning = 3,
		Info = 4,
		Debug = 5,
		Trace = 6,
		Trace1, Trace2, Trace3, Trace4, Trace5, Trace6, Trace7, Trace8, Trace9, Trace10,
		Profiler
	);

	struct Message
	{
		unsigned __int64 id;
		boost::chrono::system_clock::time_point time;
		std::string name;
		Level level;
		Context context;
		std::string message;
		std::map<std::string, std::string> tags;
		COMMON_API Message();
	};

	class Logger;
	class LogStreamPrivate;
	class LogStream;
	class COMMON_API LogStream//: private std::stringstream
	{
		friend class Logger;
		LogStreamPrivate *_d;
	

		std::stringstream& stream();

	
	public:

		LogStream(const Logger& logger, Level level);

		LogStream(const Logger& logger, Level level, const char *file, int line, const char *function);

		LogStream(const LogStream& other);

		LogStream(const LogStream&& other);

		virtual ~LogStream();

		const Logger& get_logger() const;

		LogStream& write();

		const std::string& delimiter();

		LogStream& set_delimiter(const char *delimiter);

		LogStream& set_code_source(const char *filename, int line);

		LogStream& set_code_function(const char * function);

		LogStream& add_tag(const std::string& key, const std::string& value);

		LogStream& add_block_leave();

		LogStream& join_mode(bool enable);

		LogStream& operator<< (LogStream& (*ManipFp)(LogStream& os))
		{
			return ManipFp(*this);
		};

		static LogStream& join_mode(LogStream&, bool enable);

		static LogStream& add_block_leave(LogStream&);

		static LogStream& flush(LogStream&);

		static LogStream& tag(LogStream&, const std::string& key, const std::string& value);

		LogStream& operator << (__int16 v) { stream() << v << delimiter(); return *this; }

		LogStream& operator << (__int32 v) { stream() << v << delimiter(); return *this; }

		LogStream& operator << (__int64 v) { stream() << v << delimiter(); return *this; }

		LogStream& operator << (unsigned __int16 v) { stream() << v << delimiter(); return *this; }

		LogStream& operator << (unsigned __int32 v) { stream() << v << delimiter(); return *this; }

		LogStream& operator << (unsigned __int64 v) { stream() << v << delimiter(); return *this; }

		LogStream& operator << (double v) { stream() << v << delimiter(); return *this; }

		LogStream& operator << (long double v) { stream() << v << delimiter(); return *this; }

		LogStream& operator << (float v) { stream() << v << delimiter(); return *this; }

		//#define USE_INLINE
	#ifdef USE_INLINE
		inline LogStream& operator << (const std::string& v) { _d->stream << v.c_str() << _d->delimiter; return *this; }
		inline LogStream& operator << (const char* v) { _d->stream << v << _d->delimiter; return *this; }
	#else
		LogStream& operator << (const std::string& v);
		LogStream& operator << (const char* v);
	#endif

	};


	class LoggerPrivate;
	//#define USE_SHAREDPTR
	template class __declspec(dllexport) std::shared_ptr<LoggerPrivate>;
	class COMMON_API Logger
	{
		friend class LoggerPrivate;
		friend class LogStream;
		friend class MessageHandler;
	#ifdef USE_SHAREDPTR
		std::shared_ptr<LoggerPrivate> _d;
	#else
		LoggerPrivate *_d;
	#endif
		//template class __declspec(dllexport) std::map<std::string, logger::Logger *>;
		//template class __declspec(dllexport) std::vector<logger::Logger::MessageHandler *>;
	public:
		class COMMON_API MessageHandler
		{
		public:
			virtual ~MessageHandler();
			virtual void do_handle(Message& msg) = 0;
		};
		typedef Level Level;
	private:

		Logger(Logger *parent, const std::string& name);

		virtual ~Logger();

		void send_message(Message& msg) const;

		static Logger& _get(const char *name);
	public:

		Level level() const;

		bool has_level(Level level) const;

		Logger& set_level(Level level);

		void push_level();

		void pop_level();

		const std::string& name() const;

		Logger& set_delimiter(const char *delimiter);

		Logger& set_code_source(const char *filename, int line);

		Logger& set_code_function(const char * function);
		LogStream log(Level level) const;	
		LogStream error() const;
		LogStream critical() const;
		LogStream warning() const;
		LogStream trace(unsigned int traceLevel = 0) const;
		LogStream debug() const;
		LogStream info() const;
		LogStream exception() const;


		/**
		Registers a message handler
		@param handler a handler instance
		@return none
		*/
		static void register_message_handler(MessageHandler& handler);

		/**
		Unregisters a message handler
		@param handler the handler to unregister
		@return none
		*/
		static void unregister_message_handler(MessageHandler& handler);
	
		static LogStream log(Level level, const char *file, int line, const char *function);
		static LogStream log(Level level, const char *file, int line, const char *function, Logger& logger);

		/**
		Gets the root logger
		@return reference to the root logger
		*/
		static Logger& get();

		/**
		Gets logger by various intformation used by autonaming macros
		*/
		static Logger& get(type_info *ti);
		static Logger& get(const char *fileinfo, const char *funcinfo);
		static Logger& get(const char *fileinfo, const char *funcinfo, logger::Logger& logger);

		/**
		Gets or creates a named logger
		@return reference to the named logger
		*/
		static Logger& get(const char *name);

		/**
		Sets the logging level per logger name or regular expression
		@param namePattern a specific logger name or a regex ex. "logger1" "logger1|logger2" ".*" for all
		@param level the logging level
		@return none
		*/
		static void set_level(const char *namePattern, Level level);

		static void push_level(const char *namePattern);

		static void pop_level(const char *namePattern);

		static void set_join_mode(bool enable);

		static void debug(Logger& logger);

		class Indent
		{
			friend class LogStream;
			static thread_local int indent;
		public:
			Indent()
			{
				indent += 4;
			}
			~Indent()
			{
				indent -= 4;
			}
		};

	};

	struct scoped_level
	{
		logger::Level _prevLevel;
		logger::Logger& _logger;
		scoped_level(logger::Logger& logger, logger::Level level) :
			_logger(logger)
		{
			_prevLevel = logger.level();
		}
		~scoped_level()
		{
			_logger.set_level(_prevLevel);
		}
	};

	struct scoped_global_level
	{
		std::string _pattern;
		scoped_global_level(const std::string& pattern, Level level) :
			_pattern(pattern)
		{
			Logger::push_level(pattern.c_str());
			Logger::set_level(pattern.c_str(), level);
		}
		~scoped_global_level()
		{
			Logger::pop_level(_pattern.c_str());
		}
	};
	namespace handlers {
		class ConsoleMessageHandler : public Logger::MessageHandler
		{
			bool init()
			{
				return true;
			}

			void do_handle(Message& msg)
			{
				std::cout
				<< boost::chrono::time_fmt(boost::chrono::timezone::local, "%d-%m-%Y %H:%M:%S") << msg.time << " "
				<< msg.name << " "
				//<< "Level=" << msg.level << " "
				//<< "Source" << msg.context.filename << ":" << msg.context.line << " "
				<< msg.context.function << " "
				<< msg.message
				<< std::endl;
			}
		};
	}
}

template<class F1, class F2, class F3>
struct overload_set :F1, F2, F3
{
	overload_set(F1 f1, F2 f2, F3 f3) :
		F1(f1), F2(f2), F3(f3)
	{
	}
	using F1::operator();
	using F2::operator();
	using F3::operator();
};
template<class F1, class F2, class F3>
overload_set<F1, F2, F3> overload(F1 f1, F2 f2, F3 f3)
{
	return overload_set<F1, F2, F3>(f1, f2, f3);
}

#define hidden_get_logger(...) \
	overload\
	(\
		[]() -> logger::Logger* { static logger::Logger &l = logger::Logger::get(__FILE__, __FUNCTION__); return &l;  },\
		[](logger::Logger& logger) -> logger::Logger* { return &logger;  },\
		[](logger::Logger& logger, logger::Logger&) -> logger::Logger* { return &logger;  }\
	)(__VA_ARGS__)

#define hidden_get_logstream(LOGGER, LEVEL) \
	logger::Logger::get(LOGGER)\
	.log(LEVEL)\
	.set_code_source(__FILE__, __LINE__)\
	.set_code_function(__FUNCTION__)
/*
Logging macros
*/
// general log macro 
// the first if defines just the scoped Logger ptr
#ifdef DISABLE_LOGGING
#define log_log(LEVEL, ...) qDebug() << __FUNCTION__ << ": "
#define log_join(LEVEL)
#define log_indent()
#define log_func_entry_leave()
#else

#define log_log(LEVEL, ...) \
	if(logger::Logger *l=(logger::Logger*)0x1)\
		if(l = hidden_get_logger(__VA_ARGS__)) if(l->has_level(LEVEL)) \
			logger::LogStream(*l, LEVEL, __FILE__, __LINE__, __FUNCTION__)

//			l->log(logger::Logger::Level::Debug, __FILE__, __LINE__, __FUNCTION__)

#define _log_log(LEVEL) \
	if (logger::Logger::get(__FILE__).has_level(LEVEL)) \
		hidden_get_logstream(__FILE__, LEVEL)

#define log_join(LEVEL) \
	logger::Logger *l_join_messages = hidden_get_logger(); \
	logger::LogStream ls_join_messages = l_join_messages->log(LEVEL, __FILE__, __LINE__, __FUNCTION__);\
	ls_join_messages.join_mode(true)

#define log_func_entry_leave() \
	logger::Logger::Indent __indent; \
	logger::LogStream ls_func_entry_leave = hidden_get_logstream((type_info *)&typeid(this), logger::Logger::Level::Trace); \
	ls_func_entry_leave << "leave"; \
	log_log(logger::Logger::Level::Trace) << "begin"

#define log_indent() \
	logger::Logger::Indent __indent;
#endif
#define log_set_level(LEVEL, ...) \
	(hidden_get_logger(__VA_ARGS__))->set_level(LEVEL)

#define log_set_scoped_level(PATTERN, LEVEL) \
	logger::scoped_global_level ___scoped_level___(PATTERN, logger::Logger::Level::##LEVEL)

#define log_profiler(TAG) \
	log_log(logger::Logger::Level::Error)

#define log_error() \
	log_log(logger::Logger::Level::Error)

#define log_info() \
	log_log(logger::Logger::Level::Info)

#define log_critical() \
	log_log(logger::Logger::Level::Critical)

#define log_warning() \
	log_log(logger::Logger::Level::Warning)

#define log_debug(...) \
	log_log(logger::Logger::Level::Debug, __VA_ARGS__)

#define log_trace(TRACE_LEVEL) \
	log_log((logger::Logger::Level)(static_cast<int>(logger::Logger::Level::Trace) + TRACE_LEVEL))
	
#define cpDebug(...)