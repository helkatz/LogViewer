#pragma once
#include "utils.h"
#include <qthread.h>
#include <mutex>
#include <qlogging.h>
#include <qdebug.h>
#include <qsqlerror.h>

#include <logger/Logger.h>
namespace logger {
	class SqlLogMessageHandler : public QThread, public logger::Logger::MessageHandler
{
	Q_OBJECT
	QSqlDatabase _db;
	std::unique_ptr<utils::database::SqlQuery> _sqlQuery;
	std::vector<Message> messageQueue;
	std::mutex mutex;
private slots:
	void run()
	{
		while (isRunning()) {
			QThread::msleep(500);
			{				
				std::lock_guard<std::mutex> lock(mutex);
				if (messageQueue.size() == 0)
					continue;
				std::stringstream sql;
				const std::string sqlBegin = "insert into lv_log(datetime,level,function,message) values";
				char *seperator = "";
				sql << sqlBegin;
				const int maxSqlSize = 10000;
				for (auto msg : messageQueue) {
					if ((int)sql.tellp() + msg.message.length() > maxSqlSize) {
						std::string s = sql.str();
						_sqlQuery->exec(sql.str().c_str());
						sql.str("");
						sql << sqlBegin;
						seperator = "";
					}
					std::stringstream ss;
					ss << seperator
						<< "("
						<< "'" << boost::chrono::time_fmt(boost::chrono::timezone::local, "%Y-%m-%d %H:%M:%S.%ZZZ") << msg.time << "',"
						<< msg.level << ","
						<< "'" << msg.context.function << "',"
						<< "'" << msg.message << "'"
						<< ")";
					seperator = ",";
					sql << ss.str();
				}
				if (sql.tellp() > 0) {
					std::string s = sql.str();
					bool ret = _sqlQuery->exec(sql.str().c_str());
					if (ret == false)
						ret = ret;
				}
				messageQueue.clear();
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
		_db.setPassword("helkatz");
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
		std::cout
			<< boost::chrono::time_fmt(boost::chrono::timezone::local, "%d-%m-%Y %H:%M:%S") << msg.time << " "
			<< __FUNCSIG__ << " " << __FUNCTION__ << " "
			<< msg.name << " "
			<< "Level=" << msg.level << " "
			<< "Source" << msg.context.filename << ":" << msg.context.line << " "
			<< msg.context.function << " "
			<< msg.message
			<< std::endl;
	}
};
}