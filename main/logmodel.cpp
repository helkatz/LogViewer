#include "logmodel.h"
#include "logsqlmodel.h"
#include "logfilemodel.h"
#include "logupdater.h"
#if 1

LogModel::LogModel(QObject *parent):
    QSqlQueryModel(parent),
    _rows(0)
{

}

QVariant LogModel::data(int row, int col, int role) const
{
    QModelIndex index = createIndex(row, col);
    return data(index, role);
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

	// use currentRow for all other cols than the first one its faster then query in map
	if (index.column() > 0 && _currentRow.row == index.row())
		return _currentRow.record.value(index.column());

    DataCache::const_iterator it = _dataCache.find(index.row());
    if(it == _dataCache.end()) {
		loadData(index);
		if (_currentRow.row == index.row())
			return _currentRow.record.value(index.column());
    }

    if(it != _dataCache.end()) {
		_currentRow.set(index.row(), it->second);
        return _currentRow.record.value(index.column());
    }
    return "undef index";
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    return _rows;
}

int LogModel::columnCount(const QModelIndex &parent) const
{
    return _columnsInformation.count();
}

QStringList LogModel::columns() const
{
    QStringList list;
    for(int i = 0; i < _columnsInformation.count(); i++)
        list.push_back(_columnsInformation.fieldName(i));
    return list;
}

QSqlRecord LogModel::columnsInformation() const
{
	return _columnsInformation;
}

quint64 LogModel::getFrontRow() const
{
	return 0;
}

quint64 LogModel::getBackRow() const
{
	return 0;
}

int LogModel::fetchMoreFrom(quint32 row, quint32 items, bool back)
{
	Q_UNUSED(row);
	Q_UNUSED(items);
	Q_UNUSED(back);
	return 0;
}

int LogModel::fetchToEnd()
{
	return 0;
}

int LogModel::fetchToBegin()
{
	return 0;
}

int LogModel::fetchMoreFromBegin(quint32 items)
{
	Q_UNUSED(items);
	return 0;

}

int LogModel::fetchMoreFromEnd(quint32 items)
{
	Q_UNUSED(items);
	return 0;

}

void LogModel::removeView(LogView *view)
{
	_views.removeOne(view);
	if (_views.empty()) {
		ObserverBase::removeObserver(this);
	}
}

Conditions LogModel::getQueryConditions() const
{
	return _queryConditions;
}

void LogModel::setQueryConditions(const Conditions & qc)
{
	_queryConditions = qc;
}

void LogModel::addView(LogView *view)
{
    _views.append(view);
}
#endif

Conditions::Conditions()
{

}

void Conditions::writeSettings(const QString &basePath)
{
    Settings settings(basePath);
    foreach(QString name, _hive.keys()) {
        settings.queryConditions().set(name, _hive[name]);
    }
}

void Conditions::readSettings(const QString &basePath)
{
    Settings settings(basePath);
    foreach(QString name, settings.childKeys("queryConditions")) {
        _hive[name] = settings.queryConditions().get(name);
        qDebug() << name<<"="<<_hive[name];
    }
}
