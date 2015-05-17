#include "logmodel.h"
#include "logsqlmodel.h"
#include "logfilemodel.h"
#include "logupdater.h"
void LogModel::serialize()
{

}

LogModel *LogModel::unserialize()
{
    return NULL;
}

LogModel::LogModel(QObject *parent):
    QSqlQueryModel(parent),
    _rows(0)
{

}

LogModel *LogModel::createClass(const QString &className)
{
    if(className == "LogSqlModel")
        return new LogSqlModel(NULL);
    if(className == "LogFileModel")
        return new LogFileModel(NULL);
    return NULL;
}

LogModel *LogModel::create(Conditions& queryConditions)
{
    // is there a spezielized querycondition for a logModel then create from that
    if(queryConditions.modelClass().length()) {
        LogModel *model = createClass(queryConditions.modelClass());
        model->setQueryConditions(queryConditions);
        return model;
    }

    return NULL;
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
    DataCache::const_iterator it = _dataCache.find(index.row());
    if(it == _dataCache.end()) {
        loadData(index);
        it = _dataCache.find(index.row());
    }
    if(it != _dataCache.end()) {
        const QSqlRecord& rowData = it->second;
        return rowData.value(index.column());
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

void LogModel::removeView(QObject *view)
{
    _views.removeOne(view);
    if(_views.empty()) {
        LogUpdater::instance().removeTableObserver(this);
        delete this;
    }
}

void LogModel::addView(QObject *view)
{
    _views.append(view);
}


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
