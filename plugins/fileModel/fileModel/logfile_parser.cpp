#include "logfile_parser.h"
#include "../Settings.h"
#include <Utils/utils.h>

#include <gui/ColumnizerChooser.h>

#include <QFile>
#include <QTextStream>
#include <QRegularExpressionMatch>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QThread>

#include <boost/regex.h>
#include <string>
#include <deque>
#include <iostream>
#include <algorithm>
#include <type_traits>
#include <iterator>
#include <string>
#include <stdlib.h> 

QMap<QString, Parser *> Parser::_parserMap;

Parser *Parser::findParser(const QString& fileName)
{
	auto it = _parserMap.find(fileName);
	return it != _parserMap.end() ? *it : nullptr;
}

Parser::Parser(QObject *parent, QString fileName) :
	QThread(parent),
	_fileName(fileName),
	_avgEntrySize(0),
	_linesCount(0),
	_rows(0),
	_entriesCache(100),
	_frontEntriesCache(100),
	_backEntriesCache(100),
	_virtualRows(true)
{
	Columnizer::loadAll();
	//_file.resizeBuffer(0x100000);
	connect(this, SIGNAL(entriesCountChanged(quint32)), parent, SLOT(entriesCountChanged(quint32)));
	return;
}

Parser::~Parser()
{
}

void Parser::refresh()
{
	common::File::FTime times;
	_file.getFileTime(times);
	auto err = GetLastError();
	bool fcreateChanged = memcmp(&lastFileTime.creationTime, &times.creationTime, sizeof(times.creationTime)) != 0;
	bool flastWriteChanged = memcmp(&lastFileTime.lastWriteTime, &times.lastWriteTime, sizeof(times.lastWriteTime)) != 0;
	auto fsize = _file.size();
	bool reload = fcreateChanged || fsize < _lastFileSize;
	bool amend = !fcreateChanged && flastWriteChanged && fsize > _lastFileSize;
	lastFileTime = times;

	if (flastWriteChanged && fsize == _lastFileSize || _firstLogLine.length() == 0) {
		common::File tmpFile;
		tmpFile.open(_file.getFileName());
		auto line = tmpFile.readLine();
		if (line != _firstLogLine) {
			reload = true;
		}
		_firstLogLine = line;
	}
	

	if (!reload && !amend)
		return;
	//if (_lastFileSize == _file.size() && !ftimeChanged)
	//	return;
	if (_virtualRows == false) {
		importMessages(reload);
	}
	else {
		_file.reset();
		if (_backEntriesCache.size() == 0) {
			fillBackEntriesCache();
			fillFrontEntriesCache();
		}
		else if (_file.size() < _lastFileSize) {
			_rows = calcRowCount();
		}
		else {
			_rows = calcRowCount();
			fillBackEntriesCache();
		}
	}
	emit entriesCountChanged(_rows);
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
		_rows++;
	}
	//_lastFileSize = _file.size();
}
#endif

quint64 Parser::getLinesCount()
{
	return 0;
}

QString Parser::getFileName() const
{
	return _file.getFileName().c_str();
}

bool Parser::open(const FileQueryParams& qp)
{
	auto settings = appSettings()->as<LogFileSettings>();
	if (_file.open(qp.fileName().toStdString().c_str()) == false)
		return false;
	
	_lastFileSize = _file.size();

	if (findColumnizer(qp.columnizer()) == false)
		return false;

	setFilter(qp.queryString());

	_virtualRows = _file.size() > settings.virtualRowsMinSize() * 1024 * 1024;
	if (_virtualRows == false) {
		importMessages(true);
	}
	else {
		calcRowCount();
		fillBackEntriesCache();
		fillFrontEntriesCache();
	}
	return true;
}

void Parser::fillBackEntriesCache()
{
	LogEntry entry(columnNames().size());
	quint32 itemsLeft = _backEntriesCache.maxSize();

	auto read = [&](LogEntry& entry) { return  itemsLeft-- && readLogEntry(_file, entry, -1); };

	_file.seekEnd();
	_backEntriesCache.clear();
	if (read(entry)) {
		_backEntriesCache.pushFirst(getRowCount(), entry);
		while (read(entry))
			_backEntriesCache.pushFront(entry);
	}
}

void Parser::fillFrontEntriesCache()
{
	LogEntry entry(columnNames().size());
	quint32 itemsLeft = _frontEntriesCache.maxSize();

	auto read = [&](LogEntry& entry) { return  itemsLeft-- && readLogEntry(_file, entry, 1); };

	_file.seek(0);
	_frontEntriesCache.clear();
	if (read(entry)) {
		_frontEntriesCache.pushFirst(0, entry);
		while (read(entry))
			_frontEntriesCache.pushBack(entry);
	}
}

void Parser::setFilter(const QString& filter)
{
	_filter = filter;
	QString pattern = _columnizer->searchPattern.c_str();
	QRegularExpression re("(\\w*):(.*)");
	auto matches = re.globalMatch(filter);
	while (matches.hasNext()) {
		auto m = matches.next();
		pattern.replace("COLUMNNAME_" + m.captured(1), m.captured(2));
		_filter = m.captured(2);
		
	}
	pattern.replace(QRegularExpression("COLUMNNAME_\\w*"), ".*?");
	RE2::Options re_options;
	re_options.set_dot_nl(true);
	_reSearch = std::make_shared<RE2>(pattern.toStdString(), re_options);
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
	return true;
	if (runInBackground)
		QThread::start();
	else
		run();
	return true;
}

bool Parser::findColumnizer(const QString& preferred)
{
	auto columnizers = Columnizer::find(_file);
	if (columnizers.length() == 0)
		return false;

	if (preferred.length() == 0) {
		if (columnizers.length() > 1) {
			ColumnizerChooser d;
			QStringList list;
			for (auto& c : columnizers)
				list.push_back(c.name.c_str());
			d.addList(list);
			if (d.exec() == QDialog::Accepted) {
				_columnizer = Columnizer::find(d.columnizer());
			}
		}
		else
			_columnizer = columnizers.at(0);
	}
	else {
		_columnizer = Columnizer::find(preferred);
	}

	if (!_columnizer)
		return false;

	_qre.setPattern(_columnizer->pattern.c_str());
	_qre.setPatternOptions(QRegularExpression::DotMatchesEverythingOption | QRegularExpression::MultilineOption);
	RE2::Options re_options;
	re_options.set_dot_nl(true);
	_re = std::make_shared<RE2>(_columnizer->pattern.c_str(), re_options);
	_reStart = std::make_shared<RE2>(_columnizer->startPattern.c_str(), re_options);
	
	_arguments_ptrs.resize(_columnizer->columnNames.size());
	_arguments.resize(_columnizer->columnNames.size());
	_entries.resize(_columnizer->columnNames.size());
	for (std::size_t i = 0; i < _entries.size(); i++) {
		_arguments[i] = &_entries[i];
		_arguments_ptrs[i] = &_arguments[i];
	}
	_file.reset();
	return true;
}

QString& escape(QString& str)
{
	return str
		.replace("\"", "\\\"")
		.replace("'", "''");
}
#include <type_traits>

quint64 Parser::getRowCount()
{
	//if (_rows == 0)
	//	_rows = calcRowCount();
	return _rows;
}

quint64 Parser::calcRowCount(bool extended)
{
	if (!extended && _avgEntrySize > 0) {
		return _file.size() / _avgEntrySize;
	}

	//common::File file;
	//if (file.open(_fileName.toStdString().c_str()) == false)
	//	return 0;

	quint64 checkedBytes = 0;
	quint64 checkedRows = 0;
	_avgEntrySize = 0;
	const quint32 CHECK_ROWS = 1000;

	quint64 fileSize = _file.size();
	_file.reset();
	qsrand(fileSize);
	std::string logMessage;
	LogEntry logEntry(0);
	while (_file.hasNext() && checkedRows < CHECK_ROWS) {
		if (readLogEntry(_file, logEntry) == false)
			continue;
		checkedRows++;
		quint32 entrySize = logEntry.size;

		checkedBytes += entrySize;
		_avgEntrySize = checkedBytes / checkedRows;
		if (checkedRows % 100 == 0) {
			quint32 step = fileSize / CHECK_ROWS;
			_file.seek(step * checkedRows);
			_file.readLine();
		}

	}
	if (_avgEntrySize == 0)
		return 0;
	quint64 avgRows = fileSize / _avgEntrySize;
	quint32 rows = 0;
	if (avgRows < 1000) {
		_file.reset();
		while (_file.hasNext()) {
			if (readLogEntry(_file, logEntry) == false)
				continue;
			rows++;
		}
	}
	else
		rows = avgRows;
	qDebug() << "calc rows" << rows;
	_rows = rows;
	return rows;
}


quint64 Parser::getRowFromFilePos(quint64 filePos)
{
	if (_virtualRows == false) {
		auto first = std::lower_bound(
			_entryHeads.begin(), _entryHeads.end(), filePos);
		if ((!(first == _entryHeads.end()) && !(filePos < *first))) {
			auto row = std::distance(_entryHeads.begin(), first);
			return row;
		}
	}
	return round((double)getRowCount() * ((double)filePos / _file.size()));
}

quint64 Parser::getFilePosFromRow(quint64 row)
{
	if (row == getRowCount())
		return _file.size();
	return round((double)_file.size() *((double)row / getRowCount()));
}

bool Parser::readNextLogEntry(common::File& file, LogEntry& entry)
{
	bool startMatched = false;
	bool first = true;
	quint64 firstMatchSize;
	quint64 lineStartPos;
	while (file.hasNext()) {

		lineStartPos = file.posEnd();
		if (lineStartPos == 37940) {
			printf("");
		}
		char *line = file.peekLine();
		if (*line == 0) {
			if (file.posEnd() == file.size()) {
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
			entry.filePos = file.posBegin();
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
			firstMatchSize = strlen(line);
			startMatched = true;
			file.keepBufferAtOnce(line);
		}
		else {
			if (startMatched == false)
				file.keepBufferAtOnce(true);
		}
		file.readLine();
 	}
	if (startMatched == false)
		return false;
	//QString rawMessage = file.getBufferAtOnce();
	entry.rawMessage = file.getBufferAtOnce();
	entry.rawMessage.remove(10000);
	entry.size = lineStartPos - entry.filePos;

	//auto pattern = _columnizer->pattern.substr(_columnizer->pattern.length() - _columnizer->lastPattern.length());
	//re2::RE2::Arg arg;
	//bool match = RE2::FullMatch(entry.rawMessage.toStdString().c_str()
	//	, *_re
	//	, arg);
	entry.parseColumns(_qre);
	//boost::RegEx re(_re->pattern());
	//if (re.Match(rawMessage.toStdString())) {
	//	re.Matched(0);
	//	re.Matched(1);
	//	re.Matched(4);
	//}
	//entry.rawMessage.swap(rawMessage);
	return true;
}

bool Parser::readPrevLogEntry(common::File& file, LogEntry& entry)
{
	//log_func_entry_leave();
	bool startMatched = false;
	bool first = true;
	quint64 lineEndPos;
	QVector<QString> lines;
	while (file.hasPrev()) {
		char *line = file.readPrevLine();
		if (first) {
			lineEndPos = file.posEnd();
		}
		first = false;
		lines.push_front(line);
		bool match = RE2::FullMatch(line, *_re);
		if (match == true) {
			entry.filePos = file.posBegin();
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

quint32 Parser::readLogEntry(common::File& file, LogEntry& entry, int items)
{
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
			entry.filePos = file.posBegin();
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

bool Parser::readLogEntry(common::File& file, LogEntry& entry, const QString& where, bool searchDown)
{
	// find line with condition
	char *line;
	if (searchDown)
		line = file.readLine(where.toStdString().c_str());
	else
		line = file.readPrevLine(where.toStdString().c_str());
	if (*line == 0)
		return false;
	file.seek(file.posBegin());

	// now read the whole log entry  on the found position
	readLogEntry(file, entry);
	
	if (false && _entriesCache.findInQueue(entry)) {
		return true;
	}
	_entriesCache.clear();

	// get the approximatly row for that logentry
	entry.row = getRowFromFilePos(file.posBegin());
	_entriesCache.pushFirst(entry.row, entry);
	return true;

}

int Parser::fetchToEnd(quint32 items)
{
	LogEntry curEntry(columnNames().size());
	//_file.seek(curEntry.filePos);

	_file.seekEnd();
	readLogEntry(_file, curEntry, -1);

	_entriesCache.pushFirst(_rows - 1, curEntry);
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
	LogEntry curEntry(columnNames().size());;
	_file.seek(0);
	readLogEntry(_file, curEntry, 1);
	_entriesCache.pushFirst(0, curEntry);
	return 1;
	quint32 itemsFetched = 0;
	while (itemsFetched < items) {
		readLogEntry(_entriesCache.backRow() + 1, curEntry);
		itemsFetched++;
	}
	return itemsFetched;
}

int Parser::fetchMoreUpward(quint32 row, quint32 items)
{
	//@TODO assign jst to init with right col size LogEntry should be changed so that it does not need that
	LogEntry entry = _entriesCache.back();

	// check we have the row already cached and calc how many items we should prepend
	if (_entriesCache.findInQueue(row, entry)) {
		quint32 distance = row - _entriesCache.frontRow();
		items = distance >= items ? 0 : items - distance;
		if (items > 0)
			entry = _entriesCache.front();
		_file.seek(entry.filePos);
	}
	else {
		if (std::abs(_entriesCache.distance(row)) > 100)
			_entriesCache.clear();
		_file.seek(getFilePosFromRow(row));	
		_entriesCache.setBackRow(row);
	}

	quint32 itemsFetched = readLogEntry(_file, entry, -1 * items);

	return itemsFetched;
}

int Parser::fetchMoreDownward(quint32 row, quint32 items)
{
	quint32 itemsFetched = 0;
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
		&& _frontEntriesCache.back().filePos < curEntry.filePos
		&& itemsFetched < items)
	{
		readLogEntry(_entriesCache.frontRow() - 1, curEntry);
		itemsFetched++;
	}
	_entriesCache.setFrontRow(0);
	//row += itemsFetched;
	return itemsFetched;
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
		readLogEntry(_file, entry);
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
				if (readLogEntry(_file, entry, -1) == false)
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
				if (readLogEntry(_file, entry) == false) {
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
	readLogEntry(_file, entry);
	if (entry.size == 0) {
		log_trace(5) << "RAND  size 0" << INFO1;
	}
	_entriesCache.pushFirst(row, entry);
	log_trace(5) << "RAND  " << INFO1
		<< "filePos" << filePos << "pos2row" << filePos << getRowFromFilePos(filePos)
		<< "msgIndex" << entry.entries.at(0).data();

__LReturn:
	_entriesCache.findInQueue(row, entry);
	log_trace(5) << "CACHE " << INFO1;
	return true;
}

bool Parser::importMessages(bool reload)
{
	//#define WRITE_OUT
	common::File file;
	//file.open(_fileName.toStdString());
	LogEntry entry(columnNames().size());
	if (reload || _file.size() < _lastFileSize || _entryHeads.size() == 0) {
		_file.reset();
		_entryHeads.clear();
		_rows = 0;
	} else {
		_file.seek(_lastFileSize);
	}

	while (readLogEntry(_file, entry)) {
		_entryHeads.push_back(entry.filePos);
		_rows++;
	}

	while (false && file.hasNext()) {
		quint64 filePos = file.posEnd();
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
	importMessages(true);
	return;
}


void parser::testingMain()
{
	Parser *parser;// = Parser::createParser(fileName, "");
	_ASSERT(parser);
	LogEntry entry(0);
	parser->readLogEntry(0, entry, "apache", true);
}


