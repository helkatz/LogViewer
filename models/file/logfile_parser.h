#pragma once
#include <QFile>
#include <QSharedPointer>
#include <QMutex>
#include <qsqlrecord.h>
#include <qsqlfield.h>
#include <qthread.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qregularexpression.h>
#include <re2/prog.h>
#include <re2/regexp.h>
#include "Utils/utils.h"

#include <common/File.h>
namespace parser {
	void testingMain();
}
struct LogEntry
{
	quint32 size;
	quint64 filePos;
	quint64 row;
	QString rawMessage;
	std::vector<std::string> entries;
	LogEntry(quint16 cols) :
		size(0),
		filePos(0),
		row(0),
		entries(cols)
	{
	}

	LogEntry(const LogEntry& other)
	{
		size = other.size;
		filePos = other.filePos;
		row = other.row;
		rawMessage = other.rawMessage;
		entries = other.entries;
	}

	bool getRecord(QSqlRecord& r) const
	{
		r.clear();
		for (size_t i = 0; i < entries.size(); i++) {
			QSqlField f;
			f.setValue(entries.at(i).data());
			r.append(f);
		}
		return true;
	}

	void clear()
	{
		rawMessage = "";
		row = 0;
		filePos = 0;
		size = 0;
		for (size_t i = 0; i < entries.size(); i++)
			entries.at(i).clear();
		
	}

	bool parseColumns(const QRegularExpression& re)
	{
		if (entries.size() == 0)
			return false;
		QRegularExpressionMatch match = re.match(rawMessage);
		bool ret = match.hasMatch();
		if (!ret)
			return false;
		for (size_t i = 0; i < entries.size(); i++) {
			std::string s = match.captured(i + 1).toStdString();
			entries[i] = s;
		}
		return true;
	}
};

class Queue : public std::deque < LogEntry >
{
	quint32 _maxSize;
	quint32 _backRow;
public:
	Queue(quint32 maxSize) :
		_maxSize(maxSize),
		_backRow(0)
	{

	}

	quint32 maxSize() const
	{
		return _maxSize;
	}

	bool find(LogEntry& entry)
	{
		if (entry.filePos < front().filePos || entry.filePos > back().filePos)
			return false;
		for (size_t i = 0; i < size(); i++)
			if (entry.filePos == at(i).filePos) {
				entry.row = at(i).row;
				return true;
			}
		return false;
	}
	void check()
	{
		if (size() > 2 && at(0).filePos == at(1).filePos)
			_CrtDbgBreak();
	}

	void push(LogEntry& entry, bool back = true)
	{
		auto idx = atoi(entry.entries.at(0).c_str());
		if (idx < 2000) {
			printf("");
		}
		if (back)
			push_back(entry);
		else
			push_front(entry);
		if (size() > _maxSize) {
			if (back) {
				pop_front();
			}
			else {
				pop_back();
				_backRow--;
			}
		}
		if (back)
			_backRow++;
		assert(size() <= 2 || at(0).filePos != at(1).filePos);
		//check();
	}
	void setFrontRow(quint32 row)
	{
		_backRow = row + (size() - 1);
	}

	void setBackRow(quint32 row)
	{
		_backRow = row;
	}

	void correctRows(quint32 correction)
	{
		_backRow += correction;
	}

	quint32 backRow() const
	{
		return _backRow;
	}

	quint32 frontRow() const
	{
		return _backRow - (size() - 1);
	}

	void pushBack(LogEntry& entry)
	{
		push(entry, true);
	}

	void pushFront(LogEntry& entry)
	{
		push(entry, false);
	}

	void pushFirst(quint32 row, LogEntry& entry)
	{
		clear();
		push(entry);
		_backRow = row;		
	}

	bool canPushToFront(quint32 row, quint32& requiredItems)
	{
		requiredItems = 0;
		if (row < frontRow() && frontRow() - row < maxSize()) {
			//log_trace(5) << "PREV  " << INFO1 << " seek " << frontEntry.filePos << " items " << frontEntry.row - row;
			requiredItems = frontRow() - row;
		}
		return requiredItems != 0;
	}

	bool canPushToBack(quint32 row, quint32& requiredItems)
	{
		requiredItems = 0;
		if (row > backRow() && row < backRow() + maxSize()) {
			requiredItems = row - backRow();
		}
		return requiredItems != 0;
	}

	bool findInQueue(LogEntry& entry)
	{
		return find(entry);
	}
	bool has(quint32 row) const
	{
		return row >= frontRow() && row <= backRow();
	}

	LogEntry& get(quint32 row)
	{
		return at(row - frontRow());
	}

	const LogEntry& get(quint32 row) const
	{
		return at(row - frontRow());
	}

	bool get(quint32 row, LogEntry& entry) const
	{
		if (has(row)) {
			entry = get(row);
			return true;
		}
		return false;
	}

	bool findInQueue(quint32 row, LogEntry& entry) const
	{
		// search in the workingQueue
		if (row >= frontRow() && row <= backRow()) {
			entry = at(row - frontRow());
			entry.row = row;
			return true;
		}

		return false;
	}

	void pushWorking(quint32 row, LogEntry& entry, bool back = true)
	{
		push(entry, back);
		_backRow = back ? row : row + size() - 1;
	}
};

struct Columnizer
{
	using List = QVector<Columnizer>;
	std::string name;
	std::string pattern;
	std::string startPattern;
	std::string searchPattern;
	QList<QString> columnNames;

	static Columnizer::List _columnizers;

	static void loadColumnizers();
	static Columnizer::List findColumnizer(common::File& file);
	static boost::optional<Columnizer> find(const QString& name);

};

class Parser : public QThread
{
	Q_OBJECT;
	QMutex _mutex;
	common::File _file;
	quint64 _lastFileSize;
	QString _fileName;
	QString _filter;
	QRegularExpression _qre;
	QRegularExpressionMatch _qmatch;
	std::shared_ptr<RE2> _re;			// regex for the whole logmessage pattern
	std::shared_ptr<RE2> _reStart;		// regex to detect only entry start
	std::shared_ptr<RE2> _reSearch;
	std::vector<RE2::Arg *> _arguments_ptrs;
	std::vector<RE2::Arg> _arguments;
	std::vector<std::string> _entries;

	std::vector<quint64> _entryHeads;
	uint32_t _avgEntrySize;
	quint64 _linesCount; 
	quint64 _entriesCount;
	Queue _entriesCache;
	Queue _backEntriesCache;
	Queue _frontEntriesCache;
	
	boost::optional<Columnizer> _columnizer;

protected:
	
	static QMap<QString, Parser *> _parserMap;


	bool importMessages();

	void fillBackEntriesCache();
	void fillFrontEntriesCache();

	bool Parser::readPrevLogEntry(common::File& file, LogEntry& entry);
	bool Parser::readNextLogEntry(common::File& file, LogEntry& entry);

	quint32 readLogEntry(common::File& file, LogEntry& entry, int items = 1);
	
	bool readLogEntry(common::File& file, LogEntry& entry, const QString& where, bool searchDown = true);

	quint64 calcRowCount(bool extended = false);	

	quint64 getRowFromFilePos(quint64 filePos);

	quint64 getFilePosFromRow(quint64 row);

	bool start(bool runInBackground);

	bool findColumnizer();
public:
	Parser(QObject *parent, QString fileName);

	~Parser();

	//static Parser *createParser(const QString& fileName, const QString& conditions);

	static Parser *findParser(const QString& fileName);

	const QList<QString> columnNames()
	{
		return _columnizer ? _columnizer->columnNames : QList<QString>{};
	}
	
	void refresh();
	//void updateEntriesCount();

	//void updateEntriesCount(quint32 maxRows);

	void setFilter(const QString& filter);

	bool open(const QString& fileName);

	quint64 getRowCount();

	quint64 getLinesCount();

	QString getFileName() const;


	bool readLogEntry(quint64 row, LogEntry& entry, const QString& where, bool searchDown = true);

	bool readLogEntry(quint64 row, LogEntry& entry);

	bool correctRow(quint64& row);

	quint64 getFrontRow() const
	{
		return _entriesCache.frontRow();
	}

	quint64 getBackRow() const
	{
		return _entriesCache.backRow();
	}

	int fetchToEnd(quint32 items);

	int fetchToBegin(quint32 items);

	int fetchMoreBackward(quint32 row, quint32 items);

	int fetchMoreForward(quint32 row, quint32 items);

	int fetchMoreFromBegin(quint32 items);

	int fetchMoreFromEnd(quint32 items);

signals:
	void entriesCountChanged(quint32 newCount);

private slots:
	void run();
};
