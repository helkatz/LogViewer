#include "logfilemodel.h"
#include "logfile_parser.h"
#include <gui/logview/LogView.h>
#include <core/common.h>
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

#include <qfiledialog.h>
namespace {
	void openAction()
	{
		QFileDialog d;
		QStringList fileNames = d.getOpenFileNames();
		foreach(QString fileName, fileNames) {
			FileQueryParams qp("");
			qp.fileName(fileName);
			qp.modelClass("LogFileModel");
		}
	}
}

struct LogFileModel::Private
{
	QSharedPointer<Parser> parser;
};

LogFileModel::LogFileModel(QObject *parent)
	: LogModel(parent)
	, impl_(new Private)
{
	struct A {
		int i;
	};
	struct B : public A {

	};
}

LogModel::CurrentRow& LogFileModel::loadData(quint64 index) const
{
	LogEntry logEntry(impl_->parser->columnNames().size());
	_currentRow.reset();
	if (impl_->parser->readLogEntry(index, logEntry) == false)
		return _currentRow;
	QSqlRecord r;
	logEntry.getRecord(r);
	_currentRow.set(index, r);
	//_dataCache[index.row()] = r;
	return _currentRow;
}

quint64 LogFileModel::getFrontRow() const
{
	return impl_->parser->getFrontRow();
}

quint64 LogFileModel::getBackRow() const
{
	return impl_->parser->getBackRow();
}

int LogFileModel::fetchToEnd()
{ 
	return impl_->parser->fetchToEnd(100);
};

int LogFileModel::fetchToBegin()
{ 
	return impl_->parser->fetchToBegin(100);
};

int LogFileModel::fetchMoreUpward(quint32 row, quint32 items)
{
	return impl_->parser->fetchMoreUpward(row, items);
}

int LogFileModel::fetchMoreDownward(quint32 row, quint32 items)
{
	return impl_->parser->fetchMoreDownward(row, items);
}

int LogFileModel::fetchMoreFromBegin(quint32 items)
{
	auto fetched = impl_->parser->fetchMoreFromBegin(items);
	if (fetched != 0)
		emit layoutChanged();
	return fetched;
}

int LogFileModel::fetchMoreFromEnd(quint32 items)
{
	Q_UNUSED(items);
	return 0;
}

bool LogFileModel::query()
{
	auto qp = qp_->as<FileQueryParams>();
    if (!impl_->parser) {
        impl_->parser = QSharedPointer<Parser>(new Parser(this, qp.fileName()));
		//impl_->parser->setFilter(qp.queryString());
        if (!impl_->parser->open(qp)) {
            QMessageBox::critical(0, tr("error"), tr("could not open file %1").arg(qp.fileName()));
            throw std::exception();
        }

		auto columnizerName = impl_->parser->columnizer()->name;
		if (impl_->parser->columnizer())
			qp.columnizer(columnizerName.c_str());

        int col = 0;
        foreach(const QString& name, impl_->parser->columnNames()) {
            QSqlField f;
            f.setName(name);
            _columnsInformation.append(f);
            setHeaderData(col++, Qt::Horizontal, tr("%1").arg(capitalize(name)));
        }
		_rows = impl_->parser->getRowCount();
		virtualRows_ = impl_->parser->_virtualRows;
        //ObserverFile::createObserver(this, qp.fileName());
    }
	
    
	LogEntry entry(impl_->parser->columnNames().size());
	impl_->parser->readLogEntry(0, entry); // @testing

	observer_.install(std::chrono::milliseconds{ 1000 }, [this] {
		impl_->parser->refresh();
	});
	observer_.run();
    emit layoutChanged();
    return true;
}

bool LogFileModel::queryWithCondition(QString sqlFilter, int limit)
{
	auto qp = qp_->as<FileQueryParams>();
    qp.queryString(sqlFilter);
    qp.limit(limit);
    return query();
}

QString LogFileModel::getTitle() const
{
	auto qp = qp_->as<FileQueryParams>();
    return qp.fileName();
}

void LogFileModel::observedObjectChanged(const QString& id, const int maxId)
{
#if 0
	Q_UNUSED(maxId);
	bool b = false;
	foreach(LogView *view, _views)
	{
		b |= view->followMode();
	}
    if (id == ObserverFile::createId(qc().fileName())) {
        impl_->parser->refresh();
        emit layoutChanged();
    }
#endif
}

void LogFileModel::entriesCountChanged(quint32 newCount)
{
	Q_UNUSED(newCount);
    _rows = impl_->parser->getRowCount();
    emit layoutChanged();
}

QModelIndex LogFileModel::find(const QModelIndex& fromIndex, const QStringList & columns, const QString& search, bool regex, bool down) const
{
	Q_UNUSED(regex);
	Q_UNUSED(columns);
    qDebug() << "find fromPos:" << fromIndex.row() << "dir:" << down << "where:" << search;
    QModelIndex index = fromIndex;
    LogEntry entry(impl_->parser->columnNames().size());
    if(impl_->parser->readLogEntry(fromIndex.row() + (down ? 1 : 0), entry, search, down) == false)
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