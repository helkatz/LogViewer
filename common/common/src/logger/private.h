#pragma once
#include "functionInformation.h"
#include "cache.h"

#include <ostream>
#include <sstream>
#include <memory>
#include <chrono>
#include <mutex>
#include <regex>
#include <ctime>
#include <boost/range/adaptor/reversed.hpp>

namespace logger {
	class LoggerSettings
	{
		friend class Logger;
		static std::vector<LoggerSettings> patternSettings;
		std::string namePattern;
		Logger::Level level;
	public:
		LoggerSettings()
			: level(Logger::Level::None)
		{}
		LoggerSettings(const std::string& namePattern, Logger::Level level)
			: namePattern(namePattern)
			, level(level)
		{}
		static void add(const LoggerSettings& setting)
		{
			patternSettings.insert(patternSettings.begin(), setting);
		}

		static bool find(const std::string& loggerName, LoggerSettings& setting)
		{
			for (auto it : patternSettings) {
				std::tr1::regex r(it.namePattern);
				if (std::tr1::regex_match(loggerName.begin(), loggerName.end(), r) == false)
					continue;
				setting = it;
				return true;
			}
			return false;
		}
	};
	

	template <class C>
	class ThreadLocal
	{
		C _data;
	public:
		ThreadLocal()
		{
		}

		~ThreadLocal()
		{
		}

		C& operator * ()
		{
			return _data;
		}
	};

	class LogStreamPrivate
	{
	public:
		static std::atomic<unsigned __int64> id;
		std::stringstream stream;
		std::string delimiter;
		const Logger *logger;
		Message msg;
		std::shared_ptr<LogStream> child_stream;
		int ref_count;
		bool join_mode;
		LogStreamPrivate() :
			ref_count(1),
			join_mode(false)
		{
			id++;
		}
	};

	class LoggerPrivate
	{
	public:
		static std::map<std::string, Logger *> loggers;
		static std::vector<Logger::MessageHandler *> messageHandlers;
		static std::mutex loggers_mutex;
		static Logger& rootLogger;
		static const int maxCache = 10;
		static LoggerCache<const char *> lc[maxCache + 1];
		static ThreadLocal<bool> join_mode;
		static ThreadLocal<std::vector<Message>> joinedMessages;

		static void buildJoinedMessage(Message& msg);
		//static std::map<std::string, LoggerSettings> patternSettings;
		Logger::Level level;
		std::string name;
		std::string delimiter;
		Context context;
		Logger *parent;
		std::vector<Level> levels;
	};

	void LoggerPrivate::buildJoinedMessage(Message& msg)
	{
		auto joinedMessages = &(*LoggerPrivate::joinedMessages);
		if (joinedMessages->size()) {
			for (auto it = joinedMessages->begin(); it != joinedMessages->end(); ++it) {
				msg.message += "\n\t" + it->message;
			}
			joinedMessages->clear();
		}
		*join_mode = false;
	}


	std::vector<LoggerSettings> LoggerSettings::patternSettings;
	std::atomic<unsigned __int64> LogStreamPrivate::id;
	std::map<std::string, Logger *> LoggerPrivate::loggers;
	std::vector<Logger::MessageHandler *> LoggerPrivate::messageHandlers;
	std::mutex LoggerPrivate::loggers_mutex;
	Logger& LoggerPrivate::rootLogger = Logger::get("ROOT");
	LoggerCache<const char *> LoggerPrivate::lc[maxCache + 1];
	ThreadLocal<bool> LoggerPrivate::join_mode;
	ThreadLocal<std::vector<Message>> LoggerPrivate::joinedMessages;

}