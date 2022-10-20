#include <ostream>
#include <sstream>
#include <memory>
#include <chrono>
#include <mutex>
#include <regex>
#include <ctime>

#include <common/logger.h>
#include "private.h"
namespace logger {

	std::vector<LoggerSettings> LoggerSettings::patternSettings;
	std::atomic<unsigned __int64> LogStreamPrivate::id;
	std::map<std::string, Logger*> LoggerPrivate::loggers;
	std::vector<Logger::MessageHandler*> LoggerPrivate::messageHandlers;
	std::mutex LoggerPrivate::loggers_mutex;
	Logger& LoggerPrivate::rootLogger = Logger::get("ROOT");
	LoggerCache<const char*> LoggerPrivate::lc[maxCache + 1];
	ThreadLocal<bool> LoggerPrivate::join_mode;
	ThreadLocal<std::vector<Message>> LoggerPrivate::joinedMessages;

	thread_local int Logger::Indent::indent = 0;
	const std::string& LogStream::delimiter()
	{
		return _d->delimiter;
	}

	LogStream& LogStream::set_delimiter(const char *delimiter)
	{
		_d->delimiter = delimiter;
		return *this;
	}

	LogStream& LogStream::set_code_source(const char *filename, int line)
	{
		_d->msg.context.filename = filename;
		_d->msg.context.line = line;
		return *this;
	}

	LogStream& LogStream::set_code_function(const char * function)
	{
		_d->msg.context.function = function;
		return *this;
	}

	std::stringstream& LogStream::stream()
	{
		return _d->stream;
	}

	LogStream::LogStream(const Logger& logger, Level level):
	#ifdef USE_SHARED_PTR
		_d(std::make_shared<LogStreamPrivate>())
	#else
		_d(new LogStreamPrivate())
	#endif
	{
		_d->logger = &logger;
		// inherite from logger
		_d->msg.context = logger._d->context;
		_d->delimiter = logger._d->delimiter;
		_d->msg.level = level;
	}

	LogStream::LogStream(const Logger& logger, Level level, const char *file, int line, const char *function):
	#ifdef USE_SHARED_PTR
		_d(std::make_shared<LogStreamPrivate>())
	#else
		_d(new LogStreamPrivate())
	#endif
	{
		_d->logger = &logger;
		// inherite from logger
		_d->msg.context.filename = file != nullptr ? file : "";
		_d->msg.context.line = line;
		_d->msg.context.function = function != nullptr ? function : "";
		_d->delimiter = logger._d->delimiter;
		_d->msg.level = level;
	}

	LogStream::LogStream(const LogStream& other) :
		_d(other._d)
	{
		_d->ref_count++;
	}

	LogStream::LogStream(const LogStream&& other) :
		_d(other._d)
	{
		_d->ref_count++;
	}

	const Logger& LogStream::get_logger() const
	{
		return *_d->logger;
	}

	LogStream::~LogStream()
	{
	#ifdef USE_SHARED_PTR
		if (_d.use_count() > 1)
			return;
		write();
	#else
		if (--_d->ref_count > 0)
			return;
		write();
		delete _d;
	#endif
	}

	LogStream& LogStream::join_mode(bool enable)
	{
		_d->join_mode = enable;
		_d->logger->set_join_mode(enable);
		return *this;
	}

	LogStream& LogStream::join_mode(LogStream& os, bool enable)
	{
		return os.join_mode(enable);
	}

	LogStream& LogStream::add_tag(const std::string& key, const std::string& value)
	{
		_d->msg.tags[key] = value;
		return *this;
	}

	LogStream& LogStream::add_block_leave()
	{
		_d->child_stream = std::make_shared<LogStream>(*_d->logger, _d->msg.level);
		return *_d->child_stream;
	}

	LogStream& LogStream::add_block_leave(LogStream& os)
	{
		return os.add_block_leave();
	}

	LogStream& LogStream::flush(LogStream& os)
	{
		return os.write();
	}

	LogStream& LogStream::write()
	{
		if (_d->logger->has_level(_d->msg.level) == false)
			return *this;
		if (_d->stream.tellp() <= 0)
			return *this;
		_d->msg.time = boost::chrono::system_clock::now();
		_d->msg.id = _d->id;
		_d->msg.message = std::string().insert(0, Logger::Indent::indent, ' ') + _d->stream.str();
		_d->stream.str("");
		if (_d->join_mode) {
			_d->join_mode = false;
			LoggerPrivate::buildJoinedMessage(_d->msg);
		}
		_d->logger->send_message(_d->msg);
		return *this;
	}
	#ifndef USE_INLINE
	LogStream& LogStream::operator << (const std::string& v) { stream() << v.c_str() << _d->delimiter; return *this; }
	LogStream& LogStream::operator << (const char* v) { stream() << v << _d->delimiter; return *this; }
	#endif
	Message::Message()
	{
		level = Level::None;
	}

	Logger::MessageHandler::~MessageHandler()
	{
		Logger::unregister_message_handler(*this);
	}

	Logger::Logger(Logger *parent, const std::string& name) :
	#ifdef USE_SHAREDPTR
		_d(std::make_shared<LoggerPrivate>())
	#else
		_d(new LoggerPrivate())
	#endif
	{
		_d->parent = parent;
		_d->name = name;
		_d->level = parent ? parent->_d->level : Level::None;
		_d->delimiter = parent ? parent->_d->delimiter : "";
	}

	Logger::~Logger()
	{
		for (auto it = _d->loggers.begin(); it != _d->loggers.end(); ++it) {
			delete it->second;
		}
	#ifndef USE_SHAREDPTR
		delete _d;
	#endif
	}

	void Logger::send_message(Message& msg) const
	{
		msg.name = _d->name;
		if (*_d->join_mode) {
			(*_d->joinedMessages).push_back(msg);
			return;
		}
		std::lock_guard<std::mutex> lock(_d->loggers_mutex);
		for (auto it = _d->messageHandlers.begin(); it != _d->messageHandlers.end(); ++it) {
			(*it)->do_handle(msg);
		}
	}

	/// <summary>
	/// Logs the specified level.
	/// </summary>
	/// <param name="level">The level.</param>
	/// <returns></returns>
	LogStream Logger::log(Level level) const
	{
		return LogStream(*this, level);
	}

	/// <summary>
	/// Errors this instance.
	/// </summary>
	/// <returns></returns>
	LogStream Logger::error() const
	{
		return LogStream(*this, Level::Error);
	}

	LogStream Logger::critical() const
	{
		return LogStream(*this, Level::Critical);
	}

	LogStream Logger::warning() const
	{
		return LogStream(*this, Level::Warning);
	}

	LogStream Logger::trace(unsigned int traceLevel /* = 0 */) const
	{
		unsigned int  level = std::min(static_cast<unsigned int >(Level::Trace) + traceLevel, static_cast<unsigned int >(Level::Trace10));
		return LogStream(*this, Level(level));
	}

	LogStream Logger::debug() const
	{
		return LogStream(*this, Level::Debug);
	}

	LogStream Logger::info() const
	{
		return LogStream(*this, Level::Info);
	}

	LogStream Logger::exception() const
	{
		return LogStream(*this, Level::Error);
	}

	Level Logger::level() const
	{
		return _d->level;
	}

	bool Logger::has_level(Level level) const
	{ 
		return _d->level >= level; 
	}

	Logger& Logger::set_level(Level level)
	{ 
		_d->level = level; 
		return *this;
	}

	void Logger::push_level()
	{
		_d->levels.push_back(_d->level);
	}

	void Logger::pop_level()
	{
		if (_d->levels.size()) {
			_d->level = _d->levels.back();
			_d->levels.pop_back();
		}
	}

	const std::string& Logger::name() const
	{
		return _d->name;
	}

	Logger& Logger::set_delimiter(const char *delimiter)
	{
		_d->delimiter = delimiter;
		return *this;
	}

	Logger& Logger::set_code_source(const char *filename, int line) 
	{ 
		_d->context.filename = filename; 
		_d->context.line = line; 
		return *this;
	}

	Logger& Logger::set_code_function(const char * function) 
	{ 
		_d->context.function = function; 
		return *this;
	}

	void Logger::register_message_handler(MessageHandler& handler)
	{
		std::lock_guard<std::mutex> lock(LoggerPrivate::loggers_mutex);
		LoggerPrivate::messageHandlers.push_back(&handler);
	}

	void Logger::unregister_message_handler(MessageHandler& handler)
	{
		std::lock_guard<std::mutex> lock(LoggerPrivate::loggers_mutex);
		auto& vec = LoggerPrivate::messageHandlers;
		vec.erase(std::remove(vec.begin(), vec.end(), &handler), vec.end());
	}


	LogStream Logger::log(Level level, const char *file, int line, const char *function)
	{
		return LogStream(get(file, function), level, file, line, function);
	}

	LogStream Logger::log(Level level, const char *file, int line, const char *function, Logger& logger)
	{
		return LogStream(logger, level, file, line, function);
	}


	Logger& Logger::get()
	{
		return LoggerPrivate::rootLogger;
	}

	Logger& Logger::_get(const char *name)
	{
		if (LoggerPrivate::loggers.find(name) == LoggerPrivate::loggers.end()) {
			LoggerSettings settings;
			auto logger = new Logger(&LoggerPrivate::rootLogger, name);
			if (LoggerSettings::find(name, settings)) {
				logger->set_level(settings.level);
			}
			LoggerPrivate::loggers[name] = logger;
		}
		return *LoggerPrivate::loggers[name];
	}

	Logger& Logger::get(type_info *ti)
	{
		std::lock_guard<std::mutex> lock(LoggerPrivate::loggers_mutex);
		const int maxCache = 10;
		static LoggerCache<type_info *> lc[maxCache];
		for (int i = 0; i < maxCache; i++) {
			if (lc[i].ti == ti)
				return *lc[i].logger;
			if (lc[i].ti == NULL) {
				lc[i].ti = ti;
				lc[i].logger = &Logger::_get(get_name(ti).c_str());
				return *lc[i].logger;
			}
		}
		for (int i = 0; i < maxCache; i++)
			lc[i].clear();
		lc[0].ti = ti;
		lc[0].logger = &Logger::_get(get_name(ti).c_str());
		return *lc[0].logger;
	}

	Logger& Logger::get(const char *name)
	{
		std::lock_guard<std::mutex> lock(LoggerPrivate::loggers_mutex);
		return Logger::_get(name);
	}

	Logger& Logger::get(const char *fileinfo, const char *funcinfo, Logger& logger)
	{
		return logger;
	}
	Logger& Logger::get(const char *fileinfo, const char *funcinfo)
	{
		int i;
		const char *ti;
		const char *end;
		const char *fptr;
		std::lock_guard<std::mutex> lock(LoggerPrivate::loggers_mutex);
		for (i = 0; LoggerPrivate::lc[i].ti; i++) {
			LoggerCache<const char *>& d = LoggerPrivate::lc[i];
			ti = &d.ti[d.start];
			end = &d.ti[d.end];
			fptr = funcinfo;
			while (*fptr && *ti && ti < end && *fptr == *ti) {
				fptr++;
				ti++;
			}
			if (*fptr == 0 || *fptr == ':')
				return *LoggerPrivate::lc[i].logger;
		}
	
		if (i == LoggerPrivate::maxCache) {
			for (i = 0; i < LoggerPrivate::maxCache; i++)
				LoggerPrivate::lc[i].clear();
			i = 0;
		}
	
		FunctionInformation fi(funcinfo, fileinfo);
		LoggerPrivate::lc[i].start = fi.classNameStart;
		LoggerPrivate::lc[i].end = fi.className.length();
		LoggerPrivate::lc[i].ti = funcinfo; // here we must store the whole funcsig because its a pointer
		auto logger = &Logger::_get(fi.className.c_str());
		LoggerPrivate::lc[i].logger = logger;
		std::sort(std::begin(LoggerPrivate::lc), std::end(LoggerPrivate::lc),
			 [](const LoggerCache<const char*> & a, const LoggerCache<const char *> & b) -> bool {
			return a.end > b.end;
		});
		return *logger;
	}

	void Logger::set_level(const char *namePattern, Level level)
	{
		std::lock_guard<std::mutex> lock(LoggerPrivate::loggers_mutex);
		std::tr1::regex r(namePattern);
		LoggerSettings::add(LoggerSettings{			
			namePattern,
			level
		});
		for (auto logger: LoggerPrivate::loggers) {
			LoggerSettings setting;
			if (LoggerSettings::find(logger.second->_d->name, setting) == false)
				continue;
			std::string name = logger.second->_d->name;
			if (std::tr1::regex_match(name.begin(), name.end(), r) == false)
				continue;
			logger.second->set_level(setting.level);
		}
	}

	void Logger::push_level(const char *namePattern)
	{
		std::lock_guard<std::mutex> lock(LoggerPrivate::loggers_mutex);
		std::tr1::regex r(namePattern);

		for (auto logger : LoggerPrivate::loggers) {
			LoggerSettings setting;
			if (LoggerSettings::find(logger.second->_d->name, setting) == false)
				continue;
			std::string name = logger.second->_d->name;
			if (std::tr1::regex_match(name.begin(), name.end(), r) == false)
				continue;
			logger.second->push_level();
		}
	}

	void Logger::pop_level(const char *namePattern)
	{
		std::lock_guard<std::mutex> lock(LoggerPrivate::loggers_mutex);
		std::tr1::regex r(namePattern);

		for (auto logger : LoggerPrivate::loggers) {
			LoggerSettings setting;
			if (LoggerSettings::find(logger.second->_d->name, setting) == false)
				continue;
			std::string name = logger.second->_d->name;
			if (std::tr1::regex_match(name.begin(), name.end(), r) == false)
				continue;
			logger.second->pop_level();
		}
	}
	void Logger::set_join_mode(bool enable)
	{
		*LoggerPrivate::join_mode = enable;
	}

}
