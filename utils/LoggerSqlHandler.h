#pragma once
#include "utils.h"
#include <qthread.h>
#include <mutex>
#include <qlogging.h>
#include <qdebug.h>
#include <qsqlerror.h>
#include <qsqldriver.h>
#include <logger/Logger.h>
#include <qfile.h>
//Q_DECLARE_METATYPE(boost::chrono::system_clock::time_point)
namespace logger {

	class SqlLogMessageHandler : public QThread, public logger::Logger::MessageHandler
	{
		Q_OBJECT
		QSqlDatabase _db;
		std::unique_ptr<utils::database::SqlQuery> _sqlQuery;
		std::vector<Message> messageQueue;
		std::mutex mutex;

		void writeQueue()
		{
			std::stringstream sql;
			const std::string sqlBegin = "insert into lv_log(datetime,level,function,message) values";
			char *seperator = "";
			//sql << sqlBegin;
			const int maxSqlSize = 10000;
			for (auto msg : messageQueue) {
				if ((int)sql.tellp() > 0 && (int)sql.tellp() + msg.message.length() > maxSqlSize) {
					_sqlQuery->exec((sqlBegin + " " + sql.str()).c_str());
					sql.str("");
					seperator = "";
				}
				std::stringstream ss;
				QSqlField msgField;
				msgField.setType(QVariant::String);
				msgField.setValue(msg.message.c_str());
				ss << seperator
					<< "("
					<< "'" << boost::chrono::time_fmt(boost::chrono::timezone::local) << msg.time << "',"
					<< msg.level << ","
					<< "'" << msg.context.function << "',"
					<< _db.driver()->formatValue(msgField).toStdString()
					<< ")";
				seperator = ",";
				sql << ss.str();
			}
			if (sql.tellp() > 0) {
				std::string s = sql.str();
				bool ret = _sqlQuery->exec((sqlBegin + " " + sql.str()).c_str());
				//bool ret = _sqlQuery->exec(sql.str().c_str());
				if (ret == false)
					ret = ret;
			}
			messageQueue.clear();
		}
	private slots:
		void run()
		{
			while (isRunning()) {
				QThread::msleep(500);
				{									
					if (messageQueue.size() == 0)
						continue;
					std::lock_guard<std::mutex> lock(mutex);
					writeQueue();
				}
			}
		}
	public:
		SqlLogMessageHandler():
			QThread()
		{
			moveToThread(this);
			start();
		}

		bool init()
		{
			_db = QSqlDatabase::addDatabase("QMYSQL");
			_db.setDatabaseName("log");
			_db.setHostName("127.0.0.1");
			_db.setUserName("root");
			_db.setPassword("");
			if(_db.open() == false)
				return false;
			_sqlQuery = std::unique_ptr<utils::database::SqlQuery>(new utils::database::SqlQuery(_db));
			bool ret = _sqlQuery->exec("\
				CREATE TABLE IF NOT EXISTS `lv_log` (\
				`id` INT(11) NOT NULL AUTO_INCREMENT,\
				`datetime` DATETIME(3) NOT NULL,\
				`level` VARCHAR(50) NOT NULL,\
				`function` VARCHAR(50) NOT NULL,\
				`message` LONGTEXT NOT NULL,\
				PRIMARY KEY(`id`)\
				)\
				ENGINE = InnoDB\
			");
			qDebug() << "  " << _sqlQuery->lastError().text();
			return ret;
		}

		void do_handle(Message& msg)
		{
			std::lock_guard<std::mutex> lock(mutex);
			messageQueue.push_back(msg);
			//writeQueue();
			/*std::cout
				<< boost::chrono::time_fmt(boost::chrono::timezone::local, "%d-%m-%Y %H:%M:%S") << msg.time << " "
				<< __FUNCSIG__ << " " << __FUNCTION__ << " "
				<< msg.name << " "
				<< "Level=" << msg.level << " "
				<< "Source" << msg.context.filename << ":" << msg.context.line << " "
				<< msg.context.function << " "
				<< msg.message
				<< std::endl;*/
		}
	};

	class LogfileMessageHandler : public QThread, public logger::Logger::MessageHandler
	{
		Q_OBJECT;
		QString fileName;
		std::vector<Message> messageQueue;
		std::mutex mutex;
		QFile qtf;
		private slots:
		void run()
		{
			
			while (isRunning()) {
				QThread::msleep(500);
				{
					std::lock_guard<std::mutex> lock(mutex);
					if (messageQueue.size() == 0)
						continue;
					//auto f = fopen(fileName.c_str(), "a+");
					for (auto msg : messageQueue) {
						std::stringstream ss;
						ss
							<< boost::chrono::time_point_cast<boost::chrono::nanoseconds>(msg.time)
							<< "|" << msg.level
							<< "|" << msg.context.function
							<< "|" << msg.message
							<< "\n"
							;
						qtf.write(ss.str().c_str(), ss.tellp());
						//fwrite(ss.str().c_str(), 1, ss.tellp(), f);
					}
					//fclose(f);
					messageQueue.clear();
				}
			}
		}
	public:
		LogfileMessageHandler() :
			QThread()
		{
			moveToThread(this);
			
			start();
		}

		bool init(const QString& logFile)
		{
			fileName = logFile;
			return qtf.open(fopen(fileName.toStdString().c_str(), "a+"), QIODevice::Append);
		}

		void do_handle(Message& msg)
		{
			//std::lock_guard<std::mutex> lock(mutex);
			//messageQueue.push_back(msg);

			//auto f = fopen(fileName.c_str(), "a+");
			std::stringstream ss;
			ss
				<< boost::chrono::time_point_cast<boost::chrono::nanoseconds>(msg.time)
				<< "|" << msg.level
				<< "|" << msg.context.function
				<< "|" << msg.message
				<< "\n"
				;
			qtf.write(ss.str().c_str(), ss.tellp());
			qtf.flush();
			//fwrite(ss.str().c_str(), 1, ss.tellp(), f);

		}
	};
}

//#include <qnetworkdatagram.h>
#include <qudpsocket.h>
//#include <Poco/Net/DNS.h>
#if 1

namespace logger {

	class SyslogMessageHandler : public QThread, public logger::Logger::MessageHandler
	{
		Q_OBJECT;
		std::string _host;
		std::string _logHost;
		std::vector<Message> messageQueue;
		std::mutex mutex;
		QUdpSocket _socket;
		QHostAddress _socketAddress;
		private slots:
		void run()
		{
			while (isRunning()) {
				QThread::msleep(500);
				{
					std::lock_guard<std::mutex> lock(mutex);
					if (messageQueue.size() == 0)
						continue;
					for (auto msg : messageQueue) {
						std::stringstream ss;
						ss
							<< boost::chrono::time_point_cast<boost::chrono::nanoseconds>(msg.time)
							<< "|" << msg.level
							<< "|" << msg.context.function
							<< "|" << msg.message
							<< "\n"
							;
						QByteArray data(ss.str().c_str());
						auto err = _socket.writeDatagram(data, _socketAddress, 1514);
						if (err < 0) {
							auto s = _socket.errorString().toStdString();
							std::cout << s;
						}
					}
					messageQueue.clear();
					/*
					Poco::FastMutex::ScopedLock lock(_mutex);

					if (!_open) open();

					std::string m;
					m.reserve(1024);
					m += '<';
					Poco::NumberFormatter::append(m, getPrio(msg) + _facility);
					m += '>';
					Poco::DateTimeFormatter::append(m, msg.getTime(), SYSLOG_TIMEFORMAT);
					m += ' ';
					m += _host;
					m += ' ';
					m += _program;
					m += ':';
					m += ' ';
					m += msg.getText();

					_socket.sendTo(m.data(), static_cast<int>(m.size()), _socketAddress);
					*/
				}
			}
		}
	public:
		SyslogMessageHandler() :
			QThread()
		{
			moveToThread(this);
			start();
		}

		bool init()
		{
			// reset socket for the case that it has been previously closed
			_logHost = "127.0.0.1";
			_socketAddress = QHostAddress(_logHost.c_str());

			if (_host.empty()) {
				_host = "localhost"; // DNS::thisHost().name();
				//_host = _socket.address().host().toString();
			}
			return true;
		}

		void do_handle(Message& msg)
		{
			std::lock_guard<std::mutex> lock(mutex);
			messageQueue.push_back(msg);
		}
	};
}
#endif