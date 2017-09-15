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

class File
{
	friend class Parser;
#ifdef USE_FSTREAM
	std::fstream _fs;
#else
	mutable FILE *_fs;
#endif
	QString _fileName;
	struct Buffer
	{
		quint32 size;
		char *allocated;
		char *working;
		char *keep_at_once;
		struct Cur
		{
			static char noLF;
			char *head;
			char *line_tail;
			char *line_head;
			//char *keep_at_once;
			char *lastCR;
			char *lastLF;
			Cur() :
				head(nullptr),
				line_tail(nullptr),
				line_head(nullptr),
				//keep_at_once(nullptr),
				lastCR(&noLF),
				lastLF(&noLF)
			{
			}

			Cur(const Cur& other):
				head(other.head),
				line_tail(other.line_tail),
				line_head(other.line_head),
				lastCR(other.lastCR),
				lastLF(other.lastLF)
			{

			}

			void swap(Cur& other)
			{
				Cur tmp(*this);
				*this = other;
				other = tmp;
			}

			int clearCRLF(char *buffer)
			{
				if (*buffer == '\n')
					lastLF = buffer;
				if (*(buffer - 1) == '\r')
					lastCR = buffer - 1;
				*lastLF = *lastCR = 0;
				return (lastLF == buffer?1:0) + (lastCR == (buffer - 1)?1:0);
			}

			int restoreCRLF()
			{
				if (*lastLF == ' ' || *lastCR == ' ')
					_CrtDbgBreak();
				*lastLF = '\n';
				*lastCR = '\r';
				int size = (lastCR != &noLF) + (lastLF != &noLF);
				lastLF = lastCR = &noLF;
				return size;
			}
		};

		Cur cur;
		Cur save;
		Buffer() :
			size(0),
			allocated(nullptr),
			working(nullptr),
			keep_at_once(nullptr)
		{
		}

		void saveBuffer()
		{
			save = cur;
			save.clearCRLF(save.head);
		}

		void restoreBuffer()
		{
			save.restoreCRLF();
			cur = save;			
			clearSaveBuffer();
		}

		void clearSaveBuffer()
		{
			save.restoreCRLF();
			new(&save) Cur;

			//memset(&save, 0, sizeof(save));
		}
	};
	Buffer _buf;
	
	const int eof = 1;
	const int peeked = 2;
	const int prepare_peek = 4;
	char _flags;
	char _flags_save;
	quint64 _bufferStartPos;
	quint64 _totalReaded;

	bool _isClone;
	quint32 readBuffer(bool backward = false);
	
public:
	File();

	File(const File& other)
	{
		_isClone = true;
		_fs = nullptr;
		*this = other;
	}


	~File();

	File& operator = (const File& other)
	{
		
		// other._fs;
		_fileName = other._fileName;
		_buf = other._buf;
		_flags = other._flags;
		_flags_save = other._flags_save;
		_bufferStartPos = other._bufferStartPos;
		_totalReaded = other._totalReaded;
		return *this;
	}

	void resizeBuffer(quint32 size, bool append = true);

	bool open(const QString& fileName);

	bool hasPrev() const;

	bool hasNext() const;

	quint64 size() const;

	quint64 posTail() const;

	quint64 posHead() const;

	//quint64 pos() const;

	//quint64 posLineStart() const;	

	void reset();

	void seek(quint64 pos);

	void seekEnd();

	char *readPrevLine();

	char *readLine();

	char *peekLine();

	char *readPrevLine(const char *condition);

	char *readLine(const char *condition);

	char *getCurrentLine();

	void keepBufferAtOnce(bool enable);

	void keepBufferAtOnce(char *ptr);

	char *getBufferAtOnce();

	const QString& getFileName() const { return _fileName; }
};
//namespace database
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
