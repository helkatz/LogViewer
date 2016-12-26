#include "utils.h"
#include <qsqlquery.h>

#define BUFFER_GAP 10
#define RSIZE(SIZE) SIZE + BUFFER_GAP * 2
#define WBUF(BUF) (BUF + BUFFER_GAP)

char File::Buffer::Cur::noLF;

File::File():
	_fs(nullptr),
	_bufferStartPos(0),
	_totalReaded(0),
	_flags(0),
	_isClone(false)
{
	resizeBuffer(0x10000, true);
}

File::~File()
{
	if (_fs)
		fclose(_fs);
	if (_isClone == false)
		delete[] _buf.allocated;
}

bool File::open(const QString& fileName)
{
	_fileName = fileName;
	if (_fs)
		fclose(_fs);
	log_debug() << "";
	log_trace(1) << fileName.toStdString();
	_fs = _fsopen(fileName.toStdString().c_str(), "rb", _SH_DENYNO);
	reset();

	if (!_fs) {
		log_error() << "cannot open" <<fileName;
	}
	return _fs != nullptr;
}

bool File::hasPrev() const
{
	return posTail() > 0;
}

bool File::hasNext() const
{
	return !(_flags & eof);
}

quint64 File::size() const
{
	quint64 cur = _ftelli64(_fs);
	_fseeki64(_fs, 0, SEEK_END);
	quint64 size = _ftelli64(_fs);
	_fseeki64(_fs, cur, SEEK_SET);
	return size;
}

quint64 File::posTail() const
{
	return _bufferStartPos + (_buf.cur.line_tail - _buf.working);
}

quint64 File::posHead() const
{
	return _flags & eof ? -1 : _bufferStartPos 
		+ (_buf.cur.line_head - _buf.working) 
		+ (_buf.cur.lastCR != &Buffer::Cur::noLF) 
		+ (_buf.cur.lastLF != &Buffer::Cur::noLF);
}

void File::reset()
{
	seek(0);
}

void File::seek(quint64 pos)
{
	if (pos == File::posHead())
		return;
	_flags &= ~peeked & ~eof;
	_buf.cur.restoreCRLF();
	_buf.save.restoreCRLF();
	_buf.clearSaveBuffer();
	if (pos >= _bufferStartPos && pos < _bufferStartPos + _buf.size && pos < _totalReaded) {
		_buf.cur.head = _buf.working + pos - _bufferStartPos;
		_buf.cur.line_tail = _buf.cur.head;
		_buf.cur.line_head = _buf.cur.head;
		_buf.keep_at_once = nullptr;
		return;
	}

	int e = _fseeki64(_fs, pos, SEEK_SET);
	if (e < 0) return;

	_flags = pos >= size() ? eof : 0;
	_totalReaded = pos;
	_bufferStartPos = pos;
	*_buf.working = 0;
	_buf.cur.line_tail = _buf.working;
	_buf.cur.line_head = _buf.working;
	_buf.cur.head = _buf.working;
	_buf.keep_at_once = nullptr;
}

void File::seekEnd()
{
	seek(size());
}

void File::keepBufferAtOnce(bool enable)
{
	if (enable)
		_buf.keep_at_once = _buf.cur.line_head 
			+ (_buf.cur.lastCR != &Buffer::Cur::noLF)
			+ (_buf.cur.lastLF != &Buffer::Cur::noLF);
	else
		_buf.keep_at_once = nullptr;
}

void File::keepBufferAtOnce(char *ptr)
{
	_buf.keep_at_once = ptr;
}

char *File::getBufferAtOnce()
{
	// when last line was just peek then this line may not affect the keep_at_once buffer
	// for all buf pos > working there is a 0 termination setted but this cannot be setted at the begining it would override chars of the line
	if (_bufferStartPos == 0 && _flags & peeked && _buf.working == _buf.cur.line_tail)
		return "";
	return _buf.keep_at_once;
}

void File::resizeBuffer(quint32 size, bool append)
{
	char *newBuffer = _buf.allocated;
	if (size == _buf.size)
		return;
	newBuffer = new char[RSIZE(size)];
	memset(newBuffer, 0, RSIZE(size));
	quint32 offset = 0;
	if (_buf.size) {
		if (size < _buf.size) {
			memmove(WBUF(newBuffer), _buf.working, size);
		}
		else if (append) {
			memmove(WBUF(newBuffer), _buf.working, _buf.size);

		}
		else {
			offset = size - _buf.size;
			memmove(WBUF(newBuffer) + offset, _buf.working, _buf.size);
		}
	}
	_buf.cur.head = WBUF(newBuffer) + offset + (_buf.cur.head - _buf.working);
	_buf.cur.line_tail = WBUF(newBuffer) + offset + (_buf.cur.line_tail - _buf.working);
	_buf.cur.line_head = WBUF(newBuffer) + offset + (_buf.cur.line_head - _buf.working);
	if (_buf.cur.lastLF != &Buffer::Cur::noLF) {
		_buf.cur.lastLF = WBUF(newBuffer) + offset + (_buf.cur.lastLF - _buf.working);
	}
	if (_buf.cur.lastCR != &Buffer::Cur::noLF) {
		_buf.cur.lastCR = WBUF(newBuffer) + offset + (_buf.cur.lastCR - _buf.working);
	}
	if (_buf.keep_at_once)
		_buf.keep_at_once = WBUF(newBuffer) + offset + (_buf.keep_at_once - _buf.working);
	if (_flags & (peeked | prepare_peek)) {
		_buf.save.head = WBUF(newBuffer) + offset + (_buf.save.head - _buf.working);
		_buf.save.line_tail = WBUF(newBuffer) + offset + (_buf.save.line_tail - _buf.working);
		_buf.save.line_head = WBUF(newBuffer) + offset + (_buf.save.line_head - _buf.working);
		if (_buf.save.lastLF != &Buffer::Cur::noLF) {
			_buf.save.lastLF = WBUF(newBuffer) + offset + (_buf.save.lastLF - _buf.working);
		}
		if (_buf.save.lastCR != &Buffer::Cur::noLF) {
			_buf.save.lastCR = WBUF(newBuffer) + offset + (_buf.save.lastCR - _buf.working);
		}
	}
	_buf.working = WBUF(newBuffer);
	_buf.size = size;
	if (_buf.allocated)
		delete[] _buf.allocated;
	_buf.allocated = newBuffer;	
}

quint32 File::readBuffer(bool backward)
{
	log_trace(4) << "readBuffer" << backward;
	if (backward == false) {
		char *moveFrom;
		bool bResize = false;
		while(true) {
			if (_flags & (peeked | prepare_peek))
				moveFrom = _buf.keep_at_once && _buf.keep_at_once < _buf.save.line_tail
				? _buf.keep_at_once
				: _buf.save.line_tail;
			else
				moveFrom = _buf.keep_at_once ? _buf.keep_at_once : _buf.cur.line_tail;
			if (_buf.size == _buf.cur.line_head - moveFrom) {
				resizeBuffer(_buf.size * 2, true);
				bResize = true;
				continue;
			}
			break;
		};
		quint32 bytesToMove = moveFrom != _buf.working ? _buf.cur.line_head - moveFrom : 0;
		quint32 moveOffset = moveFrom - _buf.working;
		quint32 seekOffset = (_buf.cur.line_head - moveFrom);
		quint32 sizeRead = _buf.size - seekOffset;

		if (bResize == false) {

			_bufferStartPos += moveOffset;
			if (_buf.save.line_tail) {
				_buf.save.line_tail -= moveOffset;
				_buf.save.line_head -= moveOffset;
			}
			if (_buf.keep_at_once)
				_buf.keep_at_once -= moveOffset;
			_buf.cur.line_tail -= moveOffset;
			if (_buf.cur.lastCR != &Buffer::Cur::noLF)
				_buf.cur.lastCR -= moveOffset;
			if (_buf.cur.lastLF != &Buffer::Cur::noLF)
				_buf.cur.lastLF -= moveOffset;
			if (_buf.save.lastCR != &Buffer::Cur::noLF)
				_buf.save.lastCR -= moveOffset;
			if (_buf.save.lastLF != &Buffer::Cur::noLF)
				_buf.save.lastLF -= moveOffset;

			_buf.cur.line_head = _buf.working + seekOffset;
			memmove(_buf.working, moveFrom, bytesToMove);
			*_buf.cur.line_head = 0;
		}

		_flags &= ~eof;
		quint64 readed = 0;
		if(_fseeki64(_fs, _bufferStartPos + seekOffset, SEEK_SET) == 0)
			readed = fread(_buf.cur.line_head, 1, sizeRead, _fs);
		if (readed && *_buf.cur.line_head == 0) {
			throw std::exception(QString("fatal: in File::readBuffer file %1 has \0 signs").arg(_fileName).toStdString().c_str());
		}
		if (readed == 0 && sizeRead)
			_flags |= eof;
		_buf.cur.line_head[readed] = 0;

		_totalReaded += readed;
		log_trace(4) << "readBuffer done" << readed;
		return readed;

	}
	else {
		char *moveFrom;
		bool bResize = false;
		char *keep_head = (_buf.keep_at_once > _buf.cur.line_head ? _buf.keep_at_once : _buf.cur.line_head) + 2;
		while (true) {
			if (_flags & (peeked | prepare_peek))
				moveFrom = _buf.keep_at_once && _buf.keep_at_once < _buf.save.line_tail
				? _buf.keep_at_once
				: _buf.save.line_tail;
			else
				moveFrom = _buf.keep_at_once && _buf.keep_at_once < _buf.cur.line_tail
					? _buf.keep_at_once
					: _buf.cur.line_tail;
			if (_buf.size == keep_head - moveFrom) {
				resizeBuffer(_buf.size * 2, true);
				bResize = true;
				continue;
			}
			break;
		};
		quint32 bytesToMove = keep_head - moveFrom;
		
		quint32 sizeRead = _buf.size - bytesToMove;
		if (_bufferStartPos > sizeRead)
			_bufferStartPos -= (_buf.size - (keep_head - _buf.working)); //sizeRead;
		else {
			sizeRead = _bufferStartPos;
			_bufferStartPos = 0;
		}

		quint32 bufferShift = sizeRead; // move the keeping buffer to this position to make space for the readSize
		_buf.cur.line_head += bufferShift;
		_buf.cur.line_tail += bufferShift;
		
		memmove(_buf.working + bufferShift, moveFrom, bytesToMove);
		quint64 readed = 0;
		if (_fseeki64(_fs, _bufferStartPos, SEEK_SET) == 0) {
			_totalReaded = _bufferStartPos;
			readed = fread(_buf.working, 1, sizeRead, _fs);
		}
		_buf.working[readed + bytesToMove] = 0;
		_totalReaded += readed;
		log_trace(2) << "readBuffer done" << readed;
		// when no more is readed then seek explicit to start to reset tail/head buffers
		if (readed == 0)
			seek(0);
		return readed;
	}
	return 0;
}

char *File::readPrevLine()
{
	if (_flags & peeked) {
		_flags = _flags_save;
		_buf.clearSaveBuffer();
	}
	if (hasPrev() == false) {
		_buf.cur.line_head = _buf.cur.line_tail;
		return "";
	}
	_buf.cur.restoreCRLF();
	bool first = true;
	/*
	v=_buf.cur.line_tail
	b=_buf.working
	b            v
	\r\nLine0\r\nLine1 then it have to return Line 0 no buffer eload needed
	v
	Line1 it have to read Line0 buffer reload needed
	  v
	Line1
	b             v>
	\r\nLine0\r\nLine1 then it have to return Line 1
	*/
	enum ReturnType {Undefined = -1, CurrentLine = 0, PrevLine = 1};
	ReturnType returnCurrentLine = Undefined;
	if (_buf.cur.line_tail > _buf.working)
		returnCurrentLine = *(_buf.cur.line_tail - 1) == '\n' ? PrevLine : CurrentLine;

__L001:
	int lineBreakSize = (*(_buf.cur.line_tail - 1) == '\n') + (*(_buf.cur.line_tail - 2) == '\r');
	// it has a lineHead when there is a break before the currentLine or its seeked to the end
	bool hasLineHead = lineBreakSize > 0 || posTail() == size();
	_buf.cur.line_tail -= (_flags & eof) ? 0 : lineBreakSize;
	_buf.cur.line_head = _buf.cur.line_tail;

	while (true) {
		if (_buf.cur.line_tail > _buf.working) {
			while (_buf.cur.line_tail > _buf.working && *--_buf.cur.line_tail != '\n');
			if (*(_buf.cur.line_tail) == '\n') {
				_buf.cur.line_tail++; // seek to linestart
				if (hasLineHead) {
					_buf.cur.clearCRLF(_buf.cur.line_head + lineBreakSize - 1);
					_buf.cur.line_tail = _buf.cur.line_tail;
					_flags &= ~eof;
					return _buf.cur.line_tail;
				}
				break;
			}
		}
		// start of file reached
		if (posTail() == 0) {
			break;
		}
		quint32 readed = readBuffer(true);
		if (readed <= 0)
			break;
		if (returnCurrentLine == Undefined)
			goto __L001;
	}
	char *save_keep_buf_at_once = _buf.keep_at_once;
	_buf.keep_at_once = _buf.working + ((_buf.cur.line_tail - _buf.working) / 2);
	_buf.cur.line_head = _buf.cur.line_tail;
	char *line = readLine();
	_buf.keep_at_once = save_keep_buf_at_once;
	_flags &= ~eof;
	return line;
}

char * File::peekLine()
{	
	if (_flags & peeked)
		return _buf.save.line_tail;
	_flags_save = _flags;
	char save_flags = _flags;

	_flags |= prepare_peek;
	// peek_save could be modified in readLine when new mem will be allocated
	_buf.save = _buf.cur;	
	

	char * line = readLine();

	Buffer::Cur tmp_buf = _buf.save;
	char tmp_flags = save_flags | peeked;
	
	// set termination to the end of the previous line
	_buf.save.clearCRLF(_buf.cur.line_tail - 1);
	// save here the current buf this will re assigned after the read
	_flags_save = _flags & ~prepare_peek;
	_buf.cur.swap(_buf.save);
	// now set the before the read actually buffer as current
	_flags = tmp_flags;
	return line;
}

char *File::readLine()
{
	if (_flags & peeked) {
		_buf.cur.swap(_buf.save);
		_buf.clearSaveBuffer();
		_flags = _flags_save;
		return _buf.cur.line_tail;
	}
	int restored = _buf.cur.restoreCRLF();
	bool hasLineHead = *(_buf.cur.line_head + restored - 1) == '\n';
	if (hasLineHead)
		_buf.cur.line_head = _buf.cur.line_head + restored;
	_buf.cur.line_tail = _buf.cur.line_head;
	while (true) {
		// check for > \n because most chars are true in this case
		// and that needs just one condition
		while (*_buf.cur.line_head > '\n' || *_buf.cur.line_head != '\n' && *_buf.cur.line_head)
			_buf.cur.line_head++;

		if (*_buf.cur.line_head == '\n') {
			int cleared = _buf.cur.clearCRLF(_buf.cur.line_head);
			_buf.cur.line_head -= (cleared - 1);
			return _buf.cur.line_tail;
		}
		else {
			quint32 readed = readBuffer();
			if (readed <= 0)
				break;
		}
	}
	_buf.cur.line_head = _buf.cur.line_head;
	return _buf.cur.line_tail;
}

char *strrstr(char* first, char *last, const char *needle)
{
	char *found = nullptr;
	char chSave;
	// sets 0 termination temporary
	char *term = (char *)last; chSave = *term; *term = 0;
	quint32 count = last - first;
	quint32 step;
	char *p;
	while ((last - first) / 2 > 0) {
		auto from = first;
		//count = last - first;
		step = (last - first) / 2;
		std::advance(from, step);
		if (p = strstr(from, needle)) {                 // or: if (comp(*it,val)), for version (2)
			found = p;
			//step += found - from;
			first = found;
			//count -= step + 1;
		}
		else {
			last = from;
			//count = step;
		}
	}
	*term = chSave;
	return found;
}

char *File::readPrevLine(const char *condition)
{
	if (_flags & peeked) {
		_flags = _flags_save;
		_buf.clearSaveBuffer();
	}
	quint16 conditionLength = strlen(condition);
	if (_buf.size < conditionLength) {
		resizeBuffer(conditionLength * 4);
	}

	_buf.keep_at_once = nullptr;
	while (true) {
		_buf.cur.restoreCRLF();
		const char *found = strrstr(_buf.working, _buf.cur.line_tail, condition);		
		if (found) {
			//_buf.cur.restoreCRLF();
			_buf.cur.line_tail = _buf.cur.line_head = (char *)found;
			//quint64 foundPos = posTail();
			char *line;
			// when stays at linestart then just readLine otherwise readPrevLine
			if (*(_buf.cur.line_tail - 1) == '\n')
				line = readLine();
			else
				line = readPrevLine();
			//quint32 posInLine = foundPos - posTail();
			return line;
		}
		_buf.cur.line_tail = _buf.working + conditionLength;
		_buf.cur.line_head = _buf.cur.line_tail;
		if (_buf.cur.line_head != _buf.cur.line_tail)
			_buf.cur.line_head = _buf.cur.line_tail + conditionLength;
		quint32 readed = readBuffer(true);
		if (readed <= 0)
			return "";
		continue;
	}
}

char *File::readLine(const char *condition)
{
	if (_flags & peeked) {
		_flags = _flags_save;
		_buf.clearSaveBuffer();
	}
	_buf.cur.restoreCRLF();
	quint16 conditionLength = strlen(condition);
	if (_buf.size < conditionLength) {
		resizeBuffer(conditionLength * 4);
	}
	_buf.keep_at_once = nullptr;
	while (true) {
		char *found = strstr(_buf.cur.line_head, condition);
		if (found) {			
			_buf.cur.line_tail = _buf.cur.line_head = found;
			//quint64 foundPos = posTail();
			char *line;
			if (*(_buf.cur.line_tail - 1) == '\n')
				line = readLine();
			else
				line = readPrevLine();
			return line;
		}
		if (_totalReaded && *_buf.cur.line_head) {
			_buf.cur.line_head = _buf.working + _buf.size;
			while (_buf.cur.line_head > _buf.working && *(_buf.cur.line_head - 1) != '\n')
				_buf.cur.line_head--;
			_buf.cur.line_tail = _buf.cur.line_head;
			//_buf.keep_at_once = _buf.cur.line_head;

		}
		quint32 readed = readBuffer();
		_buf.cur.line_head = _buf.working;
		if (readed <= 0)
			return "";
		continue;
	}
}

char *File::getCurrentLine()
{
	return _buf.cur.line_tail;
}



class SQLFilter
{
	QSqlDatabase _db;
	QSharedPointer<QSqlQuery> _q;
	QString _where;
	const QList<QString>& _names;
	const QList<QVariant>& _values;
public:
	SQLFilter(const QString& where, const QList<QString>& names, const QList<QVariant>& values) :
		_names(names),
		_values(values)
	{
		_db = utils::database::getDatabase("sqlite_internal", "QSQLITE");
		_db.setDatabaseName(":memory:");
		if (_db.open() == false)
			QMessageBox::information(0, "error", "could not open sqlite database");
		_q = QSharedPointer<QSqlQuery>(new QSqlQuery(_db));
		"Message like('dada') or level='debug'";
		QString from;
		foreach(QString name, names)
		{
			from += QString(",:%1 as `%1`").arg(name);
		}
		from = "select " + from.mid(1);
		QString sql = QString("select 1 as valid from (%2) where %1").arg(where).arg(from);
		if (_q->prepare(sql) == false) {
			qDebug() << "error:" << _q->lastError().text();
		}
	}
	bool execute()
	{
		int pos = 0;
		foreach(QString name, _names)
		{
			_q->bindValue(":" + name, _values.at(pos));
			pos++;
		}
		if (_q->exec() == false) {
			qDebug() << "error:" << _q->lastError().text();
		}
		return _q->first();
	}
};

QString& operator<<(QString &out, const QString& var)
{
	out += var;
	return out;
}
QString& operator<<(QString &out, const int var)
{
	out += QString("%1").arg(var);
	return out;
}

