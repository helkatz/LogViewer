#include "logsqlmodel.h"
#include "logupdater.h"
#include "utils.h"

#include <strstream>
#include <QMessageBox>
#include <QSqlQuery>
#include <QException>
#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>
#include <QCryptographicHash>
#include <QSqlError>
#include <QStringBuilder>

QString capitalize(const QString& s)
{
    return s[0].toUpper() + s.mid(1).toLower();
}

LogSqlModel::LogSqlModel(QObject *parent):
    //QSqlQueryModel(parent),
    LogModel(parent),
    _sqlQuery(NULL),
    _fromPos(0)
{
    connect(this, SIGNAL(newQuery(QString,QString)), &LogUpdater::instance(), SLOT(addTableObserver(QString,QString)));
    connect(&LogUpdater::instance(), SIGNAL(newDataReceived(QString,QString,int)), this, SLOT(dataChanged(QString,QString,int)));
    //connect(this, SIGNAL(destroyed(QObject*)), &LogUpdater::instance(), SLOT(removeTableObserver(QObject*)));
}

LogSqlModel::~LogSqlModel()
{
    qDebug()<<"~LogSqlModel()";
    if(_sqlQuery)
        delete _sqlQuery;
}

void LogSqlModel::reportError(const QString& message)
{
    QMessageBox::warning(NULL,
        tr("SQLError"),
        tr("An error occurred: ") + message);

}

void LogSqlModel::dataChanged(const QString &connectionName, const QString &table, int maxId)
{
    if(qc().connectionName() == connectionName && qc().tableName() == table) {
        updateQuery(maxId);
        emit layoutChanged();
        emit dataChanged();
    }

}

bool LogSqlModel::loadData(const QModelIndex &index) const
{
    int fromPos = index.row() - 100 > 0 ? index.row() - 100 : 0;
    int toPos = index.row() + 100;
    QString sql = QString(_query).arg(fromPos).arg(toPos);
    qDebug()<<"loadData for index "<<index.row()<<" sql "<<sql;
    QSqlQuery& q = *_sqlQuery;
    q.exec(sql);
    if(q.first()) {
        QSqlRecord& r = q.record();
        int fromId = r.value(0).toInt();
        int currentPos = fromPos;
        QString s = QString(",%1").arg(fromId);
        int toId = fromId;
        do
        {
            QSqlRecord& r = q.record();
            _dataCache[r.value(0).toInt() - 1] = q.record();
            //_dataCache[currentPos++] = q.record();
            toId = r.value(0).toInt();
            s += QString(",%1").arg(toId);

        } while(q.next());
        //qDebug()<<"ids"<<s;
        //qDebug()<<"fromId="<<fromId<<"toId"<<toId;
    }
    q.finish();
    return true;
}

void LogSqlModel::createFilterTable(const QString &name, const QString &sql)
{

}
/*
QVariant LogSqlModel::data(int row, int col, int role) const
{
    QModelIndex index = createIndex(row, col);
    return data(index, role);
}
*/
#if 0
QVariant LogSqlModel::data(const QModelIndex &index, int role) const
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
#endif
void queryRowsCount()
{

}

void LogSqlModel::writeSettings(const QString &basePath)
{
    qc().writeSettings(basePath);
}

void LogSqlModel::readSettings(const QString &basePath)
{
    qc().readSettings(basePath);
}

void LogSqlModel::queryWithCondition(QString sqlFilter, int limit)
{
    qc().queryString(sqlFilter);
    qc().limit(limit);
    query(qc());
}

void LogSqlModel::updateQuery(int maxId)
{
    qDebug()<<"updateQuery";
    if(_updateQuery.length()) {
        int limit = qc().limit();
        int toPos = maxId;
        int fromPos = _fromPos;
        int inserted = 0;
        int loops = 0;
        fromPos = toPos < limit ? 0 : toPos - limit;
        qDebug()<<"  limit="<<limit<<"fromP="<<fromPos<<"toP="<<toPos;

        while(toPos > fromPos) {
            qDebug()<<"  fetch fromP="<<fromPos<<"toP="<<toPos;
            fromPos = fromPos < _fromPos ? _fromPos : fromPos;
            QString sql = QString(_updateQuery).arg(fromPos).arg(toPos);
            qDebug()<<"  "<<sql;
            if(_sqlQuery->exec(sql) == false)
                qDebug() << "  "<<_sqlQuery->lastError().text();
            inserted += _sqlQuery->numRowsAffected();
            qDebug()<<"  inserted="<<inserted;
            toPos -= toPos > limit ? limit : toPos;
            fromPos = toPos < limit ? 0 : toPos - limit;
            loops++;
            qDebug()<<"  next fromP="<<fromPos<<"toP="<<toPos;
        }

        _fromPos = maxId + 1;
    }
    _sqlQuery->exec(_queryCount);
    if(_sqlQuery->first()) {
        _rows = _sqlQuery->value("Rows").toInt();
    };
    qDebug()<<"  rows="<<_rows;

    _sqlQuery->finish();
}

bool LogSqlModel::query(const Conditions &queryConditions)
{
    setQueryConditions(queryConditions);//dynamic_cast<const SqlQueryConditions& >(qc);
    _dataCache.clear();

    QSqlDatabase db = Utils::getDatabase(qc().connectionName());
    db = Utils::cloneDatabase(db);
    qDebug()<<"now open db";
    if(!db.open()) {
        return false;
    }
    _sqlQuery = new QSqlQuery("", db);
    QSqlQuery& q = *_sqlQuery;
    QString _queryFrom;
    QString sqlCount;
    struct ColInformation
    {
        QString name;
        QString type;
    };
    try {
        _updateQuery = "";
        _columnsInformation = db.record(qc().tableName());
        _queryFields = "";
        // build fields and find autoincrement column
        for(int col = 0; col < _columnsInformation.count(); col++) {
            QSqlField& f = _columnsInformation.field(col);
            if(f.isAutoValue())
                _autoincCol = f;
            else
                _queryFields += "," + f.name();
            setHeaderData(col, Qt::Horizontal, tr("%1").arg(capitalize(f.name())));
        }
        _queryFields = _queryFields.mid(1);
        if(_autoincCol.name().length() == 0) {
            qDebug()<<"autoincrement field missed in table "<<qc().tableName()<<" fields="<<_queryFields;
            throw std::exception("invalid table format there must be an auto_increment column");
        }
        // select last entry and set to _maxId
        _sqlQuery->exec(QString("select max(`%1`) as id from %2")
            .arg(_autoincCol.name())
            .arg(qc().tableName()));
        if(_sqlQuery->first())
            _maxId = _sqlQuery->value("id").toInt();
        _sqlQuery->finish();

        static int tmpid = 0;
        QString logtmp = QString("logtmp%1").arg(tmpid);
        // when we have speacial query conditions then we create a tmp log table
        if(qc().queryString().length() != 0) {
            QString sql;

            db.exec(sql = QString("drop temporary table if exists %1").arg(logtmp));
            db.exec(sql = QString("\
                create temporary table %1 \
                    (%2 int auto_increment primary key, syslog_id int) \
                    engine=myisam")
                .arg(logtmp)
                .arg(_autoincCol.name()));
            qDebug() << sql<<db.lastError().text();
            _updateQuery = QString("\
                insert into %5 \
                select null as %1, syslog.%1 as syslog_id \
                    from %2 as syslog where (%3) and syslog.%1>=%#1 and syslog.%1<=%#2")
                .arg(_autoincCol.name())
                .arg(qc().tableName())
                .arg(qc().queryString())
                .arg(logtmp)
                .replace("%#", "%");
            if(true /*qc.limit > 0 think about*/)
                _updateQuery += QString(" limit %1").arg(qc().limit());
            //db.exec(QString(_updateQuery);
            //qDebug() << sql<<db.lastError().text();
            _queryFrom = QString("%3 as logtmp inner join %1 as syslog on syslog.%2 = logtmp.syslog_id")
                    .arg(qc().tableName()).arg(_autoincCol.name()).arg(logtmp);
            _queryCount = QString("\
                select count(logtmp.%1) as Rows \
                from %2 as logtmp").arg(_autoincCol.name()).arg(logtmp);
        } else {
            _queryFrom = qc().tableName() + " as logtmp";
            _queryCount = QString("\
                select count(logtmp.%1) as Rows \
                from %2 as logtmp")
                .arg(_autoincCol.name())
                .arg(qc().tableName());
        }

        _query = QString("\
            select logtmp.%1, %2 \
            from %3 \
            where logtmp.%1>=%#1 and logtmp.%1<=%#2")
                .arg(_autoincCol.name()).arg(_queryFields).arg(_queryFrom)
                .replace("%#", "%");
        qDebug()<<"query "<<_query;
        updateQuery(_maxId);
        emit newQuery(qc().connectionName(), qc().tableName());
        emit layoutChanged();
        //emit QAbstractItemModel::dataChanged();
    } catch(std::exception e) {
        QMessageBox::critical(NULL, tr(""), e.what());
        return false;
    }
    return true;

}

QString LogSqlModel::getTitle()
{
    QString title = qc().connectionName() + "/" + qc().tableName();
    if(qc().queryString().length())
        title += "/" + qc().queryString();
    return title;
}

QModelIndexList LogSqlModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    qDebug()<<"match:"<<start.row()<<" value:"<<value.toString()<<" flags:"<<flags;

    int col = start.column();
    QString colName = _columnsInformation.fieldName(col);
    QString sql = QString("`%1` like('%%2%')").arg(colName).arg(value.toString());
    QModelIndex pos = find(start, sql, true);
    QModelIndexList l;
    l.append(pos);
    return l;
}

QModelIndex LogSqlModel::find(const QModelIndex& fromIndex, QString where, bool down) const
{
    qDebug()<<"find fromPos:"<<fromIndex.row()<<"dir:"<<down<<"where:"<<where;
    QString sql;
    int pos = 0;
    QModelIndex index;
    QSqlQuery& q = *_sqlQuery;
    if(down) {
        sql =  QString(_query).arg(fromIndex.row() + 2).arg(rowCount());
        sql += " and (" + where + ") limit 1";

        q.exec(sql);
        if(q.next())
            index = createIndex(q.record().value(0).toInt() - 1 , fromIndex.column());
    } else {
        int fromPos = fromIndex.row();
        int toPos = fromPos - 2;
        double stepIncrement = 1.5;
        int step = 100;
        //fromPos = fromPos - step;
        while (fromPos > 0) {
            fromPos -= fromPos > step ? step : fromPos;
            sql =  QString(_query).arg(fromPos).arg(toPos);
            sql += " and (" + where + ") ";
            sql += QString("order by %1 desc limit 1").arg(_autoincCol.name());
            q.exec(sql);
            if(q.next()) {
                index = createIndex(q.record().value(0).toInt() - 1, fromIndex.column());
                break;
            }
            step *= stepIncrement;
            toPos = fromPos;
        }
    }
    q.finish();
    return index;
}
