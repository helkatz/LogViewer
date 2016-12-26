#include "logfile_parser.h"
#include <string>

#include <re2/prog.h>
#include <re2/regexp.h>

#include "Utils/utils.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpressionMatch>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QThread>
#include <string>
#include <deque>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <iterator>


QMap<QString, Parser *> Parser::_parserMap;
QVector<Parser::ColumnizerData> Parser::_columnizerDataMap;


void Parser::loadPatternMap()
{
	Settings s;
	foreach(const QString& name, s.childGroups("columnizer"))
	{
		ColumnizerData columnizerData;
		columnizerData.name = name.toStdString();

		foreach(const QString& col, s.childGroups("columnizer/" + name + "/columns"))
		{
			Settings::columnizerClass::columnsClass& column = s.columnizer(name).columns(col);
			// when not set then set it with the first field pattern
			if (columnizerData.startPattern.length() == 0)
				columnizerData.startPattern = column.pattern().toStdString();
			columnizerData.columnNames.push_back(column.name());
			columnizerData.pattern += column.pattern().toStdString();
			QRegularExpression re("(^.*?)\\((.*?)\\)(.*)");
			QString searchPattern = column.pattern();
			searchPattern.replace(re, QString("\\1(COLUMNNAME_%1)\\3").arg(column.name()));
			columnizerData.searchPattern += searchPattern.toStdString();
		}
		//columnizerData.pattern += "$";
		_columnizerDataMap.push_back(columnizerData);

	}
	ColumnizerData columnizerData;
	columnizerData.name = "default_columnizer";
	columnizerData.columnNames.push_back("Message");
	columnizerData.pattern = "(.*)";
	columnizerData.startPattern = "(.*)";
	_columnizerDataMap.push_back(columnizerData);
}

Parser *Parser::findParser(const QString& fileName)
{
	auto it = _parserMap.find(fileName);
	return it != _parserMap.end() ? *it : nullptr;
}

Parser::Parser(QObject *parent, QString fileName) :
	QThread(parent),
	_fileName(fileName),
	_re(nullptr),
	_linesCount(0),
	_entriesCount(0),
	_entriesCache(100),
	_frontEntriesCache(1),
	_backEntriesCache(1)

{
	loadPatternMap();
	_file.resizeBuffer(0x100000);
	connect(this, SIGNAL(entriesCountChanged(quint32)), parent, SLOT(entriesCountChanged(quint32)));
	return;
}

Parser::~Parser()
{
}

void Parser::refresh()
{
	_file.reset();
	_file.seek(_lastFileSize + 1);
	LogEntry entry(columnNames().size());
	while (readLogEntry(_file, entry, false)) {
		//_entriesCount++;
		if (_entriesCache.backRow() == _entriesCount - 1) {
			_entriesCache.pushBack(entry);
			_entriesCache.setBackRow(_entriesCount - 1);
		}
		_backEntriesCache.pushBack(entry);
	}
	_file.reset();
	_lastFileSize = _file.size();
}
#if 0
void Parser::updateEntriesCount(quint32 maxRows)
{
	//_file.reset();
	_file.seek(getFilePosFromRow(maxRows));
	LogEntry entry(0);
	if (readLogEntry(_file, entry, true)) {
		_entriesCount++;
	}
	//_lastFileSize = _file.size();
}
#endif
quint64 Parser::getEntriesCount()
{
	if (_entriesCount == 0)
		_entriesCount = calcRows();
	return _entriesCount;
}

quint64 Parser::getLinesCount()
{
	return 0;
}

QString Parser::getFileName() const
{
	return _file.getFileName();
}

bool Parser::open(const QString& fileName)
{
	if (_file.open(fileName.toStdString().c_str()) == false)
		return false;
	_lastFileSize = _file.size();
	findColumnizer();
	_entriesCount = calcRows();
	
	LogEntry entry(columnNames().size());
	quint32 itemsLeft = _backEntriesCache.maxSize();
	quint32 row = _entriesCount;
	_file.seekEnd();
	bool first = true;
	while (itemsLeft-- && readLogEntry(_file, entry, false, -1)) {
		if (row = _entriesCount)
			_backEntriesCache.pushFirst(row, entry);
		else
			_backEntriesCache.pushFront(entry);
		entry.clear();
	}

	itemsLeft = _frontEntriesCache.maxSize();
	row = 0;
	_file.seek(0);
	while (itemsLeft-- && readLogEntry(_file, entry, false, 1)) {
		if (row = 0)
			_frontEntriesCache.pushFirst(row, entry);
		else
			_frontEntriesCache.pushBack(entry);
		entry.clear();
	}
	return true;
}

void Parser::setFilter(const QString& filter)
{
	_filter = filter;
	QString pattern = _columnizerData.searchPattern.c_str();
	QRegularExpression re("(\\w*)[=]*'(.*?)'[ ]*(and|or|[ ]|$)");
	auto matches = re.globalMatch(filter);
	while (matches.hasNext()) {
		auto m = matches.next();
		pattern.replace("COLUMNNAME_" + m.captured(1), m.captured(2));
		_filter = m.captured(2);
		
	}
	pattern.replace(QRegularExpression("COLUMNNAME_\\w*"), ".*?");
	RE2::Options re_options;
	re_options.set_dot_nl(true);
	_reSearch = new RE2(pattern.toStdString(), re_options);
}

bool Parser::start(bool runInBackground)
{
	if (!_file.open(_fileName.toStdString().c_str())) {
		QMessageBox::information(0, "error", "could not open file");
		return false;
	}
	_lastFileSize = _file.size();
	if (findColumnizer() == false) {
		QMessageBox::information(0, "error", "could not find a columnizer");
		return false;
	}
	/*
	_db = Utils::getDatabase("sqlite_internal", "QSQLITE");
	//_db.setDatabaseName("db.sqlite");
	_db.setDatabaseName(":memory:");
	if (_db.open() == false)
	QMessageBox::information(0, "error", "could not open sqlite database");
	*/
	return true;
	if (runInBackground)
		QThread::start();
	else
		run();
	return true;
}

bool Parser::findColumnizer()
{
	int tryLines = 20;
	while (_file.hasNext() && --tryLines > 0) {
		QString line = _file.readLine();
		foreach(ColumnizerData pattern, _columnizerDataMap)
		{
			QRegularExpression re(pattern.pattern.c_str());
			QRegularExpressionMatch match = re.match(line);
			if (match.hasMatch() == false)
				continue;
			if (_re)
				delete _re;
			
			_columnizerData = pattern;

			_qre.setPattern(_columnizerData.pattern.c_str());
			_qre.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
			RE2::Options re_options;
			re_options.set_dot_nl(true);
			_re = new RE2(_columnizerData.pattern.c_str(), re_options);
			_reStart = new RE2(_columnizerData.startPattern.c_str(), re_options);
	
			_arguments_ptrs.resize(pattern.columnNames.size());
			_arguments.resize(pattern.columnNames.size());
			_entries.resize(pattern.columnNames.size());
			for (std::size_t i = 0; i < _entries.size(); i++) {
				_arguments[i] = &_entries[i];
				_arguments_ptrs[i] = &_arguments[i];
			}
			_file.reset();
			return true;
		}
	}
	_file.reset();
	return false;
}

QString& escape(QString& str)
{
	return str
		.replace("\"", "\\\"")
		.replace("'", "''");
}
#include <type_traits>
quint64 Parser::calcRows()
{
	return 10000;
	File file;
	//return INT32_MAX / 100;
	if (file.open(_fileName.toStdString().c_str()) == false)
		return 0;

	quint64 checkedBytes = 0;
	quint64 checkedRows = 0;
	quint64 avgEntrySize = 0;
	const quint32 CHECK_ROWS = 1000;

	quint64 fileSize = file.size();
	qsrand(fileSize);
	std::string logMessage;
	LogEntry logEntry(0);
	while (file.hasNext() && checkedRows < CHECK_ROWS) {
		if (readLogEntry(file, logEntry, true) == false)
			continue;
		checkedRows++;
		quint32 entrySize = logEntry.size;

		checkedBytes += entrySize;
		avgEntrySize = checkedBytes / checkedRows;
		if (checkedRows % 4 == 0) {
			quint32 step = fileSize / CHECK_ROWS;
			file.seek(step * checkedRows);
			file.readLine();
		}

	}
	quint64 avgRows = fileSize / (checkedBytes / checkedRows);
	quint32 rows = 0;
	if (avgRows < 1000) {
		file.reset();
		while (file.hasNext()) {
			if (readLogEntry(file, logEntry, true) == false)
				continue;
			rows++;
		}
	}
	else
		rows = avgRows;

	qDebug() << "calc rows" << rows;
	return rows;
}

quint64 Parser::getRowFromFilePos(quint64 filePos)
{
	return round((double)getEntriesCount() * ((double)filePos / _file.size()));
}

quint64 Parser::getFilePosFromRow(quint64 row)
{
	if (row == _entriesCount)
		return _file.size();
	return round((double)_file.size() *((double)row / getEntriesCount()));
}

bool Parser::readNextLogEntry(File& file, LogEntry& entry)
{
	bool startMatched = false;
	bool first = true;
	quint64 lineStartPos;
	while (file.hasNext()) {

		lineStartPos = file.posHead();
		char *line = file.peekLine();
		if (*line == 0) {
			if (file.posHead() == file.size()) {
				file.readLine();
				continue;
			}
		}
		bool match = RE2::FullMatch(line, *_re);
		if (match == false && first) {
			while (file.hasPrev() && match == false) {
				line = file.readPrevLine();
				match = RE2::FullMatch(line, *_re);
			}
			entry.filePos = file.posTail();
			startMatched = true;
			file.keepBufferAtOnce(line);
			//line = file.readLine();
			first = false;
			continue;
		}
		first = false;

		if (match == true) {
			if (startMatched)
				break;
			entry.filePos = lineStartPos;
			startMatched = true;
		}
		else {
			if (startMatched == false)
				file.keepBufferAtOnce(true);
		}
		file.readLine();
	}
	if (startMatched == false)
		return false;
	entry.rawMessage = file.getBufferAtOnce();
	entry.size = lineStartPos - entry.filePos;
	entry.parseColumns(_qre);
	return true;
}

bool Parser::readPrevLogEntry(File& file, LogEntry& entry)
{
	//log_func_entry_leave();
	bool startMatched = false;
	bool first = true;
	quint64 lineEndPos;
	QVector<QString> lines;
	while (file.hasPrev()) {
		char *line = file.readPrevLine();
		if (first) {
			lineEndPos = file.posHead();
		}
		first = false;
		lines.push_front(line);
		bool match = RE2::FullMatch(line, *_re);
		if (match == true) {
			entry.filePos = file.posTail();
			startMatched = true;
			break;
		}
	}
	if (startMatched == false)
		return false;
	entry.rawMessage = "";
	foreach(const QString& line, lines)
	{
		entry.rawMessage += line + "\n";
	}
	entry.size = lineEndPos - entry.filePos;
	entry.parseColumns(_qre);
	return true;
}

quint32 Parser::readLogEntry(File& file, LogEntry& entry, bool matchOnly, int items)
{
	Q_UNUSED(matchOnly);
	bool startMatched = false;
	bool first = true;
	

	char *line = nullptr;
	file.keepBufferAtOnce(true);
	int itemsLeft = abs(items);
	while (itemsLeft) {
		if (_filter.length()) {
			if (items < 0)
				line = file.readPrevLine(_filter.toStdString().c_str());
			else
				line = file.readLine(_filter.toStdString().c_str());
			if (!*line)
				break;
			if (RE2::FullMatch(line, *_reSearch) == false)
				continue;
			entry.filePos = file.posTail();
			file.seek(entry.filePos);
			startMatched = true;
			file.keepBufferAtOnce(line);
		}
		if (items < 0)
			startMatched = readPrevLogEntry(file, entry);
		else
			startMatched = readNextLogEntry(file, entry);
		if (startMatched == false)
			break;
		itemsLeft--;
	}
	return abs(items) - itemsLeft;
}


bool Parser::readLogEntry(quint64 row, LogEntry& entry, const QString& where, bool searchDown)
{
	readLogEntry(row, entry);
	_file.seek(entry.filePos);
	return readLogEntry(_file, entry, where, searchDown);
}

bool Parser::readLogEntry(File& file, LogEntry& entry, const QString& where, bool searchDown)
{
	// find line with condition
	char *line;
	if (searchDown)
		line = file.readLine(where.toStdString().c_str());
	else
		line = file.readPrevLine(where.toStdString().c_str());
	if (*line == 0)
		return false;
	file.seek(file.posTail());

	// now read the whole log entry  on the found position
	readLogEntry(file, entry, false);
	
	if (_entriesCache.findInQueue(entry)) {
		return true;
	}
	_entriesCache.clear();

	// get the approximatly row for that logentry
	entry.row = getRowFromFilePos(file.posTail());
	_entriesCache.pushFirst(entry.row, entry);
	return true;

}

int Parser::fetchToEnd(quint32 items)
{
	LogEntry curEntry = _backEntriesCache.back();
	//_file.seek(curEntry.filePos);
	_file.seekEnd();
	readLogEntry(_file, curEntry, true, -1);
	_entriesCache.pushFirst(_entriesCount - 1, curEntry);
	return 1;
	quint32 itemsFetched = 0;
	while (itemsFetched < items) {
		readLogEntry(_entriesCache.backRow() - 1, curEntry);
		itemsFetched++;
	}
	return itemsFetched;
}

int Parser::fetchToBegin(quint32 items)
{
	LogEntry curEntry = _frontEntriesCache.back();
	_file.seek(0);
	readLogEntry(_file, curEntry, true, 1);
	_entriesCache.pushFirst(0, curEntry);
	return 1;
	quint32 itemsFetched = 0;
	while (itemsFetched < items) {
		readLogEntry(_entriesCache.backRow() + 1, curEntry);
		itemsFetched++;
	}
	return itemsFetched;
}

int Parser::fetchMoreFrom(quint32 row, quint32 items, bool back)
{
	quint32 itemsFetched = 0;
	if (back == false) {		
		LogEntry curEntry = _entriesCache.front();
		if (_entriesCache.findInQueue(row, curEntry)) {
			quint32 distance = row - _entriesCache.frontRow();
			items = distance > items ? 0 : items - distance;
		}
		else {

		}
		// when cur front entry actually far from  frontqueue then increase front/back row
		// and then read entries back to fill up
		_entriesCache.setFrontRow(items);
		while (_entriesCache.front().filePos > 0 
			   &&_frontEntriesCache.back().filePos < curEntry.filePos 
			   && itemsFetched < items) 
		{
			readLogEntry(_entriesCache.frontRow() - 1, curEntry);
			itemsFetched++;
		}
		_entriesCache.setFrontRow(0);
		//row += itemsFetched;
		return itemsFetched;
	}
	else {
		LogEntry curEntry = _entriesCache.back();
		if (_entriesCache.findInQueue(row, curEntry)) {
			quint32 distance = _entriesCache.backRow() - row;
			items = distance > items ? 0 : items - distance;
		}
		else {
			// when cur row not reached to the end gap then do nothing
			if (row < _backEntriesCache.frontRow())
				return 0;
			_entriesCache.clear();
			for (auto e: _backEntriesCache)
				_entriesCache.push_back(e);
			_entriesCache.setFrontRow(_backEntriesCache.frontRow());
		}
		
		// when cur front entry actually far from  frontqueue then increase front/back row
		// and then read entries back to fill up
		_entriesCache.setBackRow(_entriesCache.backRow() - items);
		while (_entriesCache.back().filePos > 0
			   && _backEntriesCache.front().filePos > curEntry.filePos
			   && itemsFetched < items) {
			readLogEntry(_entriesCache.backRow() + 1, curEntry);
			itemsFetched++;
		}
		_entriesCache.setBackRow(getEntriesCount());
		//row += itemsFetched;
		//getRowFromFilePos()
		return itemsFetched;
	}
}

int Parser::fetchMoreFromBegin(quint32 items)
{
	LogEntry curEntry = _entriesCache.front();
	quint32 itemsFetched = 0;
	// when cur front entry actually far from  frontqueue then increase front/back row
	// and then read entries back to fill up
	_entriesCache.setFrontRow(_entriesCache.frontRow() + items);
	while (_frontEntriesCache.back().filePos < curEntry.filePos && itemsFetched < items) {
		readLogEntry(_entriesCache.frontRow() - 1, curEntry);
		itemsFetched++;
	}
	// when not all items could be fetched then correct rows finaly
	_entriesCache.setFrontRow(_entriesCache.frontRow() - (items - itemsFetched));
	/*
	if (itemsFetched < items)
		_entriesCache.setFrontRow(itemsFetched);
	else
		_entriesCache.setFrontRow(_entriesCache.frontRow() + itemsFetched - items);
*/
	// when cur front entry actually in frontqueue
	while (_frontEntriesCache.back().filePos > curEntry.filePos && itemsFetched < items) {

	}
	return itemsFetched;
}

int Parser::fetchMoreFromEnd(quint32 items)
{
	return 0;
}

bool Parser::correctRow(quint64& row)
{
	// check if row in frontQueue so when u scroll up to the beginning of the log
	if (_frontEntriesCache.has(row)) {
		LogEntry& tmpEntry = _frontEntriesCache.get(row);
		LogEntry& curEntry = _entriesCache.front();
		int items = 0;
		while (_frontEntriesCache.back().filePos < curEntry.filePos) {			
			log_trace(5) << "new row+1" << row;
			readLogEntry(row, curEntry);
			items++;
		}
		_entriesCache.correctRows(items);
		if(_frontEntriesCache.back().filePos < curEntry.filePos)
		if (curEntry.filePos > tmpEntry.filePos) {
			row += 1;
			log_trace(5) << "new row+1" << row;
			_entriesCache.correctRows(1);
			readLogEntry(row, curEntry);
		}
		return true;
	}

	if (_backEntriesCache.has(row)) {
		LogEntry& tmpEntry = _backEntriesCache.get(row);
		if (_entriesCache.has(row)) {
			LogEntry& curEntry = _entriesCache.get(row);
			if (curEntry.filePos < tmpEntry.filePos) {
				row -= 1;
				_entriesCache.correctRows(-1);
			}
		}
		return true;
	}
	return false;
}

#define INFO1 "row" <<":"<< row+1<<entry.entries.at(0).data() << "front" << _entriesCache.frontRow() <<":"<<_entriesCache.front().entries.at(0).data()<< "back" << _entriesCache.backRow()<<":"<<_entriesCache.back().entries.at(0).data()

bool Parser::readLogEntry(quint64 row, LogEntry& entry)
{
	if (row < _entryHeads.size()) {
		_file.seek(_entryHeads.at(row));
		readLogEntry(_file, entry, false);
		return true;
	}
	if (_entriesCache.size()) {
		LogEntry& frontEntry = _entriesCache.front();
		LogEntry& backEntry = _entriesCache.back();

		if (_entriesCache.findInQueue(row, entry)) {
			log_trace(5) << "CACHE " << INFO1;
			return true;
		}

		// returns also entriesCount needed to fill up from last entry in queue to this row
		// so when last row is 10 and row to get is 15 then we have to fill from 10 to 15 in the cache
		// entriesCount would then be 5
		quint32 requiredItems;
		if (_entriesCache.canPushToFront(row, requiredItems)) {
			LogEntry entry = _entriesCache.front();
			log_trace(5) << "PREV  " << INFO1 << " seek " << frontEntry.filePos << " items " << _entriesCache.frontRow() - row;			
			while (requiredItems-- > 0) {
				_file.seek(entry.filePos - 2);
				if (readLogEntry(_file, entry, false, -1) == false)
					break;
				_entriesCache.pushFront(entry);
			}
			goto __LReturn;
		}
		else if (_entriesCache.canPushToBack(row, requiredItems)) {
			LogEntry entry = _entriesCache.back();
			log_trace(5) << "NEXT  " << INFO1 << " seek " << backEntry.filePos + backEntry.size << " items " << requiredItems;
			_file.seek(backEntry.filePos + backEntry.size);
			while (requiredItems-- > 0) {
				if (readLogEntry(_file, entry, false) == false) {
					break;
				}
				log_trace(5) << "  -readed pos " << entry.filePos << " size " << entry.size << " msgidx " << entry.entries.at(0).data();
				_entriesCache.pushBack(entry);
			}
			goto __LReturn;
		}
	}
	quint64 filePos = getFilePosFromRow(row);
	_file.seek(filePos);
	readLogEntry(_file, entry, false);
	_entriesCache.pushFirst(row, entry);
	log_trace(5) << "RAND  " << INFO1
		<< "filePos" << filePos << "pos2row" << filePos << getRowFromFilePos(filePos)
		<< "msgIndex" << entry.entries.at(0).data();

__LReturn:
	_entriesCache.findInQueue(row, entry);
	log_trace(5) << "CACHE " << INFO1;
	return true;
}

bool Parser::importMessages()
{
	//#define WRITE_OUT
	File file;
	file.open(_fileName);

	while (!file.hasNext()) {
		quint64 filePos = file.posHead();
		char *line = file.readLine();
		_linesCount++;
		bool match = RE2::FullMatch(line, *_re);
		if (match == true) {
			_entryHeads.push_back(filePos);
#ifdef WRITE_OUT
			static char outline[4096];
			static FILE *_fileOut = fopen(QString(_fileName + "lined").toStdString().c_str(), "w+");
			entriesCount++;
			sprintf(outline, "%u %s", entriesCount, line);
			fwrite(outline, 1, strlen(outline), _fileOut);

		}
		else
			fwrite(line, 1, strlen(line), _fileOut);
#else
	}
#endif
}
	return true;
}

void Parser::run()
{
	importMessages();
	return;
}


void parser::testingMain()
{
	Parser *parser;// = Parser::createParser(fileName, "");
	_ASSERT(parser);
	LogEntry entry(0);
	parser->readLogEntry(0, entry, "apache", true);
}