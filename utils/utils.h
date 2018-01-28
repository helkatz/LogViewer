#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "main/include/settings.h"
#include <qglobal.h>
#include <QSqlDatabase>
#include <qsqlquery.h>
#include <QSqlError>
#include <QMessageBox>
#include <QTranslator>
#include <fstream>
#include <time.h>
#include <logger/Logger.h>

//#include "Utils/Logger.h"
namespace logger {
	inline logger::LogStream& operator << (logger::LogStream& ls, const QString& v)
	{
		ls << v.toStdString();
		return ls;
	}
}
namespace utils {
	void testingMain();
}

class MySqlDatabase: public QSqlDatabase
{
public:
    MySqlDatabase(QString type):
      QSqlDatabase(type)
    {

    }
};

namespace utils
{
	namespace database {
		class SqlQuery : public QSqlQuery
		{
			QSqlDatabase _db;
			using QSqlQuery::QSqlQuery;
		public:
			explicit SqlQuery(QSqlDatabase db) :
				QSqlQuery(db),
				_db(db)
			{

			}
			bool exec(const QString& query)
			{
				int reconnect_tries = 3;
				bool ret = false;
			__LRetry:
				ret = QSqlQuery::exec(query);
				if (ret == false && reconnect_tries-- > 0) {
					qDebug() << lastError().type() << lastError().number() << lastError().text();
					if (_db.isOpen() == false
						|| lastError().type() == QSqlError::ConnectionError
						|| lastError().type() == QSqlError::StatementError) {
						// try to reopen
						_db.open();
						goto __LRetry;
					}
				}
				return ret;
			}
		};
		static QSqlDatabase cloneDatabase(const QSqlDatabase& other)
		{
			static int clone = 0;
			return QSqlDatabase::cloneDatabase(other, QString("__cloned__db%1").arg(++clone));
		}

		static QSqlDatabase getDatabase(const QString& name, const QString& driver, const QString& host, const QString& database, const QString& username, const QString& password)
		{
			QSqlDatabase db = QSqlDatabase::addDatabase(driver, name);
			db.setDatabaseName(database);
			db.setHostName(host);
			db.setUserName(username);
			db.setPassword(password);
			if (db.open() == false) {
				QMessageBox::critical(NULL,
					QObject::tr("Unable to open database"),
					db.lastError().text());
				QSqlDatabase::removeDatabase(name);
				return QSqlDatabase();
			}
			return db;
		}

		static QSqlDatabase getDatabase(const QString& name, const QString& driver = "")
		{
			QSqlDatabase db;
			if (QSqlDatabase::contains(name) == false) {
				Settings settings;
				db = QSqlDatabase::addDatabase(
					driver.length() ? driver : settings.connections().database(name).driver(),
					name);
				db.setDatabaseName(settings.connections().database(name).database());
				db.setHostName(settings.connections().database(name).host());
				db.setUserName(settings.connections().database(name).username());
				db.setPassword(settings.connections().database(name).password());
			}
			else
				db = QSqlDatabase::database(name);
			if (db.lastError().isValid())
				QMessageBox::critical(NULL,
				QObject::tr("Unable to open database"),
				db.lastError().text());
			return db;
		}

	}
	static std::string ReplaceAll(std::string str, const std::string& from, const std::string& to)
	{
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
		}
		return str;
	}

	static QDateTime parseTime(const char *fromString, const char *format);
	static char *strptime(const char *s, const char *format, struct tm *tm);
};


class DateTime
{
	struct tm tm;
	long usec;
public:
	DateTime();
	void parseTime(const char *fromString, const char *format);
	std::string toString(const char * format);
};
QString& operator<<(QString &out, const QString& var);
QString& operator<<(QString &out, const int var);
