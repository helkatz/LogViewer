#include "logupdater.h"
#include "logview.h"
#include "logstashmodel.h"

#include "common.h"
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

//#define qDebug() if(0) QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO).debug()


LogStashModel::LogStashModel(QObject *parent):
    LogModel(parent)
{
    
}

int LogStashModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
    if (_parser)
        return _parser->getEntriesCount();
    return 0;
}

QVariant LogStashModel::data(const QModelIndex &index, int role) const
{
	//log_func_entry_leave()
    if (role != Qt::DisplayRole)
        return QVariant();
	//log_trace(2) << "currentRow=" << index.row();
	quint64 row = index.row();
    if (currentRow.row != row || index.column() == 0) {
		/*
		if (_rows - row < 20 || row < 20) {
			if (_parser->correctRow(row)) {
				log_trace(5) << "row corrected old" << index.row() << "new" << row;
			}
		}*/
        LogEntry logEntry(_parser->columnNames().size());
        if (_parser->readLogEntry(row, logEntry) == false)
            return false;

        //emit const_cast<LogStashModel *>(this)->layoutChanged();
        logEntry.getRecord(currentRow.r);
        log_trace(2) << "currentRow=" << index.row()+1 << "msgindex" << logEntry.entries.at(0).data();
        currentRow.row = row;
		if (index.row() != row) {
			//const_cast<LogStashModel *>(this)->insertRows(0, 1);
			//emit const_cast<LogStashModel *>(this)->setModifiedPos(row);
		}
			
    }
    return currentRow.r.value(index.column());
}

quint64 LogStashModel::getFrontRow() const
{
	return _parser->getFrontRow();
}

quint64 LogStashModel::getBackRow() const
{
	return _parser->getBackRow();
}

int LogStashModel::fetchToEnd()
{ 
	return _parser->fetchToEnd(100);
};

int LogStashModel::fetchToBegin()
{ 
	return _parser->fetchToBegin(100);
};

int LogStashModel::fetchMoreFrom(quint32 row, quint32 items, bool back)
{
	return _parser->fetchMoreFrom(row, items, back);
}

int LogStashModel::fetchMoreFromBegin(quint32 items)
{
	auto fetched = _parser->fetchMoreFromBegin(items);
	if (fetched != 0)
		emit layoutChanged();
	return fetched;
}

int LogStashModel::fetchMoreFromEnd(quint32 items)
{
	Q_UNUSED(items);
	return 0;
}

bool LogStashModel::query(const Conditions &queryConditions)
{
    if (!_parser) {
        _parser = QSharedPointer<Parser>(new Parser(this, qc().fileName()));
        if (!_parser->open(qc().fileName().toStdString().c_str())) {
            QMessageBox::critical(0, tr("error"), tr("could not open file %1").arg(qc().fileName()));
            throw std::exception();
        }

        int col = 0;
        foreach(const QString& name, _parser->columnNames()) {
            QSqlField f;
            f.setName(name);
            _columnsInformation.append(f);
            setHeaderData(col++, Qt::Horizontal, tr("%1").arg(capitalize(name)));
        }

        _rows = _parser->getEntriesCount();

        ObserverFile::createObserver(this, qc().fileName());
    }
    _parser->setFilter(queryConditions.queryString());
	LogEntry entry(_parser->columnNames().size());
	_parser->readLogEntry(0, entry); // @testing
    emit layoutChanged();
    return true;
}

bool LogStashModel::queryWithCondition(QString sqlFilter, int limit)
{
    qc().queryString(sqlFilter);
    qc().limit(limit);
    return query(qc());
}

void LogStashModel::writeSettings(const QString &basePath)
{
    qc().writeSettings(basePath);
}

void LogStashModel::readSettings(const QString &basePath)
{
    qc().readSettings(basePath);
}

QString LogStashModel::getTitle() const
{
    return _parser->getFileName();
}

void LogStashModel::observedObjectChanged(const QString& id, const int maxId)
{
	Q_UNUSED(maxId);
	bool b = false;
	foreach(LogView *view, _views)
	{
		b |= view->followMode();
	}
    if (b && id == ObserverFile::createId(qc().fileName())) {
        _parser->refresh();
        emit layoutChanged();
    }
}

void LogStashModel::entriesCountChanged(quint32 newCount)
{
	Q_UNUSED(newCount);
    _rows = _parser->getEntriesCount();
    emit layoutChanged();
}

QModelIndex LogStashModel::find(const QModelIndex& fromIndex, const QStringList & columns, const QString& search, bool regex, bool down) const
{
	Q_UNUSED(regex);
	Q_UNUSED(columns);
    qDebug() << "find fromPos:" << fromIndex.row() << "dir:" << down << "where:" << search;
    QModelIndex index = fromIndex;
    LogEntry entry(_parser->columnNames().size());
    if(_parser->readLogEntry(fromIndex.row() + (down ? 1 : 0), entry, search, down) == false)
        QMessageBox::information(0, "information", "text not found");
    else
        index = createIndex(entry.row, fromIndex.column());
    return index;
}

QModelIndex LogStashModel::index(int row, int column, const QModelIndex &parent) const
{
    auto index = QSqlQueryModel::index(row, column, parent);
    if (index.isValid())
        return index;
    if (row >= rowCount()) {
        //auto delta = row - rowCount();
        //const_cast<LogStashModel *>(this)->updateRowCount(row);
        //emit const_cast<LogStashModel *>(this)->layoutChanged();
    }
    return QSqlQueryModel::index(rowCount() - 1, column, parent);

}