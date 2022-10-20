#pragma once
#include <QFile>
#include <QSharedPointer>
#include <QMutex>
#include <qsqlrecord.h>
#include <qsqlfield.h>
#include <qthread.h>
#include <deque>
#include <qfile.h>
#include <qtextstream.h>
#include <qregularexpression.h>
//#include <re2/prog.h>
#include <re2/re2.h>
#include <utils/utils.h>
#include <common/File.h>
#include "Columnizer.h"
#include "logfilemodel.h"
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
		auto lineBreakPos = rawMessage.indexOf("\n");
		const auto hasMulitpleLines = lineBreakPos >= 0;
		QString firstLine;
		QString rest;
		if (hasMulitpleLines) {
			firstLine = rawMessage.mid(0, lineBreakPos);
			rest = rawMessage.mid(lineBreakPos);
		}
		else {
			firstLine = rawMessage;
		}
		QRegularExpressionMatch match = re.match(firstLine);
		bool ret = match.hasMatch();
		if (!ret)
			return false;
		for (size_t i = 0; i < entries.size(); i++) {
			std::string s = match.captured(static_cast<int>(i + 1)).toStdString();
			entries[i] = s;
		}
		entries[entries.size() - 1] += rest.toStdString();
		return true;
	}
};

class Queue : public std::deque < LogEntry >
{
	quint32 _maxSize;
	quint64 _backRow;
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
		if (size() == 0)
			return false;
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
	void setFrontRow(quint64 row)
	{
		_backRow = row + (size() - 1);
	}

	void setBackRow(quint64 row)
	{
		_backRow = row;
	}

	void correctRows(quint64 correction)
	{
		_backRow += correction;
	}

	qint32 distance(quint64 row) const
	{
		if (row < frontRow())
			return row - frontRow();
		if (row > backRow())
			return row - backRow();
		return 0;
	}

	quint64 backRow() const
	{
		return _backRow;
	}

	quint64 frontRow() const
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

	void pushFirst(quint64 row, LogEntry& entry)
	{
		clear();
		push(entry);
		_backRow = row;		
	}

	bool canPushToFront(quint64 row, quint32& requiredItems)
	{
		requiredItems = 0;
		if (row < frontRow() && frontRow() - row < maxSize()) {
			//log_trace(5) << "PREV  " << INFO1 << " seek " << frontEntry.filePos << " items " << frontEntry.row - row;
			requiredItems = frontRow() - row;
		}
		return requiredItems != 0;
	}

	bool canPushToBack(quint64 row, quint32& requiredItems)
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

	bool has(quint64 row) const
	{
		return row >= frontRow() && row <= backRow();
	}

	LogEntry& get(quint64 row)
	{
		return at(row - frontRow());
	}

	const LogEntry& get(quint32 row) const
	{
		return at(row - frontRow());
	}

	bool get(quint64 row, LogEntry& entry) const
	{
		if (has(row)) {
			entry = get(row);
			return true;
		}
		return false;
	}

	bool findInQueue(quint64 row, LogEntry& entry) const
	{
		// search in the workingQueue
		if (row >= frontRow() && row <= backRow()) {
			entry = at(row - frontRow());
			entry.row = row;
			return true;
		}

		return false;
	}

	void pushWorking(quint64 row, LogEntry& entry, bool back = true)
	{
		push(entry, back);
		_backRow = back ? row : row + size() - 1;
	}
};

class Parser : public QThread
{
	Q_OBJECT;
	friend class LogFileModel;
	QMutex _mutex;
	common::File _file;
	quint64 _lastFileSize;
	QString _firstLogLine;
	common::File::FTime lastFileTime;
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
	quint64 _rows;
	Queue _entriesCache;
	Queue _backEntriesCache;
	Queue _frontEntriesCache;
	
	boost::optional<Columnizer> _columnizer;
	bool _virtualRows;
protected:
	
	static QMap<QString, Parser *> _parserMap;


	bool importMessages(bool reload);

	void fillBackEntriesCache();
	void fillFrontEntriesCache();

	bool Parser::readPrevLogEntry(common::File& file, LogEntry& entry);
	bool Parser::readNextLogEntry(common::File& file, LogEntry& entry);

	quint32 readLogEntry(common::File& file, LogEntry& entry, int items = 1);
	
	bool readLogEntry(common::File& file, LogEntry& entry, const QString& where, bool searchDown = true);

	quint64 getRowFromFilePos(quint64 filePos);

	quint64 getFilePosFromRow(quint64 row);

	bool start(bool runInBackground);

	bool findColumnizer(const QString& preferred = "");
public:
	Parser(QObject *parent, QString fileName);

	~Parser();

	//static Parser *createParser(const QString& fileName, const QString& conditions);

	static Parser *findParser(const QString& fileName);

	const boost::optional<Columnizer>& columnizer() const
		{ return _columnizer; }

	const QList<QString> columnNames()
		{ return _columnizer ? _columnizer->columnNames : QList<QString>{}; }
	
	void refresh();
	//void updateEntriesCount();

	//void updateEntriesCount(quint32 maxRows);

	void setFilter(const QString& filter);

	bool open(const FileQueryParams& qp);

	quint64 calcRowCount(bool extended = false);

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

	int fetchMoreUpward(quint32 row, quint32 items);

	int fetchMoreDownward(quint32 row, quint32 items);

	int fetchMoreFromBegin(quint32 items);

	int fetchMoreFromEnd(quint32 items);

signals:
	void entriesCountChanged(quint32 newCount);

private slots:
	void run();
};
