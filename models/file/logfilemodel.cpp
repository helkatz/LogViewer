#include "logfilemodel.h"
#include <logupdater.h>
#include <logview.h>
#include <common.h>
#include <utils/utils.h>

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

namespace {
	auto reg = LogModelFactory::Register<LogFileModel>("LogFileModel");
}
LogFileModel::LogFileModel(QObject *parent):
    LogModel(parent)
{
}

LogModel::CurrentRow& LogFileModel::loadData(uint64_t index) const
{
	LogEntry logEntry(parser_->columnNames().size());
	_currentRow.reset();
	if (parser_->readLogEntry(index, logEntry) == false)
		return _currentRow;
	QSqlRecord r;
	logEntry.getRecord(r);
	_currentRow.set(index, r);
	//_dataCache[index.row()] = r;
	return _currentRow;
}

quint64 LogFileModel::getFrontRow() const
{
	return parser_->getFrontRow();
}

quint64 LogFileModel::getBackRow() const
{
	return parser_->getBackRow();
}

int LogFileModel::fetchToEnd()
{ 
	return parser_->fetchToEnd(100);
};

int LogFileModel::fetchToBegin()
{ 
	return parser_->fetchToBegin(100);
};

int LogFileModel::fetchMoreUpward(quint32 row, quint32 items)
{
	return parser_->fetchMoreUpward(row, items);
}

int LogFileModel::fetchMoreDownward(quint32 row, quint32 items)
{
	return parser_->fetchMoreDownward(row, items);
}

int LogFileModel::fetchMoreFromBegin(quint32 items)
{
	auto fetched = parser_->fetchMoreFromBegin(items);
	if (fetched != 0)
		emit layoutChanged();
	return fetched;
}

int LogFileModel::fetchMoreFromEnd(quint32 items)
{
	Q_UNUSED(items);
	return 0;
}

bool LogFileModel::query(const Conditions &queryConditions)
{
    if (!parser_) {
        parser_ = QSharedPointer<Parser>(new Parser(this, qc().fileName()));

        if (!parser_->open(qc().fileName().toStdString().c_str())) {
            QMessageBox::critical(0, tr("error"), tr("could not open file %1").arg(qc().fileName()));
            throw std::exception();
        }

        int col = 0;
        foreach(const QString& name, parser_->columnNames()) {
            QSqlField f;
            f.setName(name);
            _columnsInformation.append(f);
            setHeaderData(col++, Qt::Horizontal, tr("%1").arg(capitalize(name)));
        }

        _rows = parser_->getRowCount();		
        //ObserverFile::createObserver(this, qc().fileName());
    }
    parser_->setFilter(queryConditions.queryString());
	LogEntry entry(parser_->columnNames().size());
	parser_->readLogEntry(0, entry); // @testing

	observer_.install(std::chrono::milliseconds{ 1000 }, [this] {
		parser_->refresh();
	});

    emit layoutChanged();
    return true;
}

bool LogFileModel::queryWithCondition(QString sqlFilter, int limit)
{
    qc().queryString(sqlFilter);
    qc().limit(limit);
    return query(qc());
}

void LogFileModel::writeSettings(const QString &basePath)
{
    qc().writeSettings(basePath);
}

void LogFileModel::readSettings(const QString &basePath)
{
    qc().readSettings(basePath);
}

QString LogFileModel::getTitle() const
{
    return parser_->getFileName();
}

void LogFileModel::observedObjectChanged(const QString& id, const int maxId)
{
	Q_UNUSED(maxId);
	bool b = false;
	foreach(LogView *view, _views)
	{
		b |= view->followMode();
	}
    if (id == ObserverFile::createId(qc().fileName())) {
        parser_->refresh();
        emit layoutChanged();
    }
}

void LogFileModel::entriesCountChanged(quint32 newCount)
{
	Q_UNUSED(newCount);
    _rows = parser_->getRowCount();
    emit layoutChanged();
}

QModelIndex LogFileModel::find(const QModelIndex& fromIndex, const QStringList & columns, const QString& search, bool regex, bool down) const
{
	Q_UNUSED(regex);
	Q_UNUSED(columns);
    qDebug() << "find fromPos:" << fromIndex.row() << "dir:" << down << "where:" << search;
    QModelIndex index = fromIndex;
    LogEntry entry(parser_->columnNames().size());
    if(parser_->readLogEntry(fromIndex.row() + (down ? 1 : 0), entry, search, down) == false)
        QMessageBox::information(0, "information", "text not found");
    else
        index = createIndex(entry.row, fromIndex.column());
    return index;
}

QModelIndex LogFileModel::index(int row, int column, const QModelIndex &parent) const
{
    auto index = QSqlQueryModel::index(row, column, parent);
    if (index.isValid())
        return index;
    if (row >= rowCount()) {
        //auto delta = row - rowCount();
        //const_cast<LogFileModel *>(this)->updateRowCount(row);
        //emit const_cast<LogFileModel *>(this)->layoutChanged();
    }
    return QSqlQueryModel::index(rowCount() - 1, column, parent);

}