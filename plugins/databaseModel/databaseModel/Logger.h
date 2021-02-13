#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QThread>

#include <common/Logger.h>
#include "SqlQuery.h"
class SqlLogMessageHandler : public QThread, public logger::Logger::MessageHandler
{
	Q_OBJECT
	QSqlDatabase _db;
	std::unique_ptr<SqlQuery> _sqlQuery;
	std::vector<logger::Message> messageQueue;
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
		_sqlQuery = std::unique_ptr<SqlQuery>(new SqlQuery(_db));
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

	void do_handle(logger::Message& msg)
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