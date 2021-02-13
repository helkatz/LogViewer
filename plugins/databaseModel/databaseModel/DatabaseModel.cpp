#include "DatabaseModel.h"
//#include <Utils/utils.h>

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

#include "SqlQuery.h"
#include "Helper.h"

namespace database {
    class SqlQuery : public QSqlQuery
    {
    	QSqlDatabase _db;
    	using QSqlQuery::QSqlQuery;
    public:
    	explicit SqlQuery(QSqlDatabase db) :
    		QSqlQuery(db),
    		_db(db)
    	{

    	}
    	bool exec(const QString& query)
    	{
    		int reconnect_tries = 3;
    		bool ret = false;
    	__LRetry:
    		ret = QSqlQuery::exec(query);
    		if (ret == false && reconnect_tries-- > 0) {
    			qDebug() << lastError().type() << lastError().number() << lastError().text();
    			if (_db.isOpen() == false
    				|| lastError().type() == QSqlError::ConnectionError
    				|| lastError().type() == QSqlError::StatementError) {
    				// try to reopen
    				_db.open();
    				goto __LRetry;
    			}
    		}
    		return ret;
    	}
    };
    static QSqlDatabase cloneDatabase(const QSqlDatabase& other)
    {
    	static int clone = 0;
    	return QSqlDatabase::cloneDatabase(other, QString("__cloned__db%1").arg(++clone));
    }

    static QSqlDatabase getDatabase(const QString& name, const QString& driver, const QString& host, const QString& database, const QString& username, const QString& password)
    {
    	QSqlDatabase db = QSqlDatabase::addDatabase(driver, name);
    	db.setDatabaseName(database);
    	db.setHostName(host);
    	db.setUserName(username);
    	db.setPassword(password);
    	if (db.open() == false) {
    		QMessageBox::critical(NULL,
    			QObject::tr("Unable to open database"),
    			db.lastError().text());
    		QSqlDatabase::removeDatabase(name);
    		return QSqlDatabase();
    	}
    	return db;
    }

    static QSqlDatabase getDatabase(const QString& name, const QString& driver = "")
    {
        DatabaseSettings settings;
    	QSqlDatabase db;
    	if (QSqlDatabase::contains(name) == false) {
            auto s = settings.connections(name);
    		db = QSqlDatabase::addDatabase(
    			driver.length() ? driver : s.driver(),
    			name);
    		db.setDatabaseName(s.database());
    		db.setHostName(s.host());
    		db.setUserName(s.username());
    		db.setPassword(s.password());
    	}
    	else
    		db = QSqlDatabase::database(name);
    	if (db.lastError().isValid())
    		QMessageBox::critical(NULL,
    		QObject::tr("Unable to open database"),
    		db.lastError().text());
    	return db;
    }

}
LogDatabaseModel::LogDatabaseModel(QObject *parent):
    LogModel(parent),
    _fromPos(0),
    _maxId(0)
{
}

LogDatabaseModel::~LogDatabaseModel()
{
    qDebug()<<"~LogDatabaseModel()";
}

void LogDatabaseModel::reportError(const QString& message)
{
    QMessageBox::warning(NULL,
        tr("SQLError"),
        tr("An error occurred: ") + message);

}

void LogDatabaseModel::observedObjectChanged(const QString &id, int maxId)
{
    //if(id == ObserverTable::createId(qp().connectionName(), qp().tableName())) {
    //    updateQuery(maxId);
    //    emit layoutChanged();
    //    emit dataChanged();
    //}

}

LogModel::CurrentRow& LogDatabaseModel::loadData(quint64 index) const
{
    int fromPos = index - 100 > 0 ? index - 100 : 0;
    int toPos = index + 100;
    QString sql = QString(_query).arg(fromPos).arg(toPos);
    _sqlQuery->exec(sql);
	_currentRow.reset();
    if(_sqlQuery->first()) {
        QSqlRecord& r = _sqlQuery->record();
        int fromId = r.value(0).toInt();
        QString s = QString(",%1").arg(fromId);
        int toId = fromId;
        do {
            QSqlRecord& r = _sqlQuery->record();
			auto curRow = r.value(0).toInt() - 1;
			if (curRow == index) {
				_currentRow.set(curRow, r);
			}
            _dataCache[curRow] = r;
            toId = r.value(0).toInt();
            s += QString(",%1").arg(toId);
        } while(_sqlQuery->next());
    }
    _sqlQuery->finish();
	return _currentRow;
}

/*
QVariant LogDatabaseModel::data(int row, int col, int role) const
{
    QModelIndex index = createIndex(row, col);
    return data(index, role);
}
*/

bool LogDatabaseModel::queryWithCondition(QString sqlFilter, int limit)
{
    qp().queryString(sqlFilter);
    qp().limit(limit);
    return query();
}

void LogDatabaseModel::updateQuery(int maxId)
{
    //qDebug()<<"updateQuery";
    if(_updateQuery.length()) {
        int limit = qp().limit();
        int toPos = maxId;
        int fromPos = _fromPos;
        int inserted = 0;
        int loops = 0;
        fromPos = toPos < limit ? 0 : toPos - limit;
        //qDebug()<<"  limit="<<limit<<"fromP="<<fromPos<<"toP="<<toPos;

        while(toPos > fromPos) {
            //qDebug()<<"  fetch fromP="<<fromPos<<"toP="<<toPos;
            fromPos = fromPos < _fromPos ? _fromPos : fromPos;
            QString sql = QString(_updateQuery).arg(fromPos).arg(toPos);
            //qDebug()<<"  "<<sql;
            if(_sqlQuery->exec(sql) == false)
                //qDebug() << "  "<<_sqlQuery->lastError().text();
            inserted += _sqlQuery->numRowsAffected();
            qDebug()<<"  inserted="<<inserted;
            toPos -= toPos > limit ? limit : toPos;
            fromPos = toPos < limit ? 0 : toPos - limit;
            loops++;
            //qDebug()<<"  next fromP="<<fromPos<<"toP="<<toPos;
        }

        _fromPos = maxId + 1;
    }
    _sqlQuery->exec(_queryCount);
    if(_sqlQuery->first()) {
        _rows = _sqlQuery->value("Rows").toInt();
    };
    //qDebug()<<"  rows="<<_rows;

    _sqlQuery->finish();
}

bool LogDatabaseModel::query()
{
    //setQueryConditions(queryConditions);
    _dataCache.clear();
    
    QSqlDatabase db;
    db = helper::getDatabase(qp().connectionName());
	db = helper::cloneDatabase(db);
    //qDebug() << "now open db";
    if (!db.open()) {
        return false;
    }

	_sqlQuery = std::unique_ptr<SqlQuery>(new SqlQuery(db));

    QString _queryFrom;
    QString sqlCount;
    struct ColInformation
    {
        QString name;
        QString type;
    };
    try {
        _updateQuery = "";
        _columnsInformation = db.record(qp().tableName());
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
            qDebug()<<"autoincrement field missed in table "<<qp().tableName()<<" fields="<<_queryFields;
            QMessageBox::critical(0, tr("error"), tr("autoincrement field missed in table %1").arg(qp().tableName()));
            return false;
            //throw std::exception("invalid table format there must be an auto_increment column");
        }
        // select last entry and set to _maxId
        _sqlQuery->exec(QString("select max(`%1`) as id from %2")
            .arg(_autoincCol.name())
            .arg(qp().tableName()));
        if(_sqlQuery->first())
            _maxId = _sqlQuery->value("id").toInt();
        _sqlQuery->finish();

        static int tmpid = 0;
        QString logtmp = qp().tableName();

        // when we have special query conditions then we create a tmp log table
        if(qp().queryString().length() != 0) {
            logtmp = QString("logtmp%1").arg(tmpid++);
            QString sql;
            if (db.driverName() == "QMYSQL") {
                db.exec(sql = QString("drop temporary table if exists %1").arg(logtmp));
                db.exec(sql = QString("\
                    create temporary table %1 \
                        (%2 int auto_increment primary key, syslog_id int) \
                        engine=myisam")
                    .arg(logtmp)
                    .arg(_autoincCol.name()));
            }
            else {
                db.exec(sql = QString("drop table if exists %1").arg(logtmp));
                db.exec(sql = QString("\
                    create table %1 \
                        (%2 integer primary key, syslog_id int)")
                    .arg(logtmp)
                    .arg(_autoincCol.name()));

            }

            qDebug() << sql<<db.lastError().text();
            _updateQuery = QString("\
                insert into %4 \
                select null as %1, syslog.%1 as syslog_id \
                    from %2 as syslog where (%3) and syslog.%1>=%#1 and syslog.%1<=%#2")
                .arg(_autoincCol.name())
                .arg(qp().tableName())
                .arg(qp().queryString())
                .arg(logtmp)
                .replace("%#", "%");
            if(false /*qc.limit > 0 think about*/)
                _updateQuery += QString(" limit %1").arg(qp().limit());
            //db.exec(QString(_updateQuery);
            //qDebug() << sql<<db.lastError().text();
            _queryFrom = QString("%3 as logtmp inner join %1 as syslog on syslog.%2 = logtmp.syslog_id")
                    .arg(qp().tableName()).arg(_autoincCol.name()).arg(logtmp);

        } else {
            _queryFrom = qp().tableName() + " as logtmp";
        }

        _queryCount = QString("\
            select count(*) as Rows \
            from %2 as logtmp").arg(logtmp);

        _query = QString("\
            select logtmp.%1, %2 \
            from %3 \
            where logtmp.%1>=%#1 and logtmp.%1<=%#2")
                .arg(_autoincCol.name()).arg(_queryFields).arg(_queryFrom)
                .replace("%#", "%");
        qDebug()<<"query "<<_query;
        updateQuery(_maxId);


        observer_.install(std::chrono::milliseconds{ 1000 }, [this] {

            });

        emit layoutChanged();
    } catch(std::exception e) {
        QMessageBox::critical(NULL, tr(""), e.what());
        return false;
    }
    return true;

}

QString LogDatabaseModel::getTitle() const
{
    QString title = qp().connectionName() + "/" + qp().tableName();
    if(qp().queryString().length())
        title += "/" + qp().queryString();
    return title;
}

QModelIndexList LogDatabaseModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    Q_UNUSED(hits)
    Q_UNUSED(role)
    
    qDebug()<<"match:"<<start.row()<<" value:"<<value.toString()<<" flags:"<<flags;

    int col = start.column();
    QString colName = _columnsInformation.fieldName(col);
    QString sql = QString("`%1` like('%%2%')").arg(colName).arg(value.toString());
    //return start;
    
    //QModelIndex pos = find(start, sql, true);
    QModelIndexList l;
    l.append(start);
    return l;
}

QModelIndex LogDatabaseModel::find(const QModelIndex& fromIndex, const QStringList & columns, const QString& search, bool regex, bool down) const
{
	Q_UNUSED(regex);
	Q_UNUSED(columns);
    qDebug() << "find fromPos:" << fromIndex.row() << "dir:" << down << "where:" << search;
    QString sql;
    QModelIndex index;
    if(down) {
        sql =  QString(_query).arg(fromIndex.row() + 2).arg(rowCount());
        sql += " and (" + search + ") limit 1";

		_sqlQuery->exec(sql);
		if (_sqlQuery->next())
			index = createIndex(_sqlQuery->record().value(0).toInt() - 1, fromIndex.column());
    } else {
        int fromPos = fromIndex.row();
        int toPos = fromPos - 2;
        double stepIncrement = 1.5;
        int step = 100;
        //fromPos = fromPos - step;
        while (fromPos > 0) {
            fromPos -= fromPos > step ? step : fromPos;
            sql =  QString(_query).arg(fromPos).arg(toPos);
            sql += " and (" + search + ") ";
            sql += QString("order by %1 desc limit 1").arg(_autoincCol.name());
			_sqlQuery->exec(sql);
			if (_sqlQuery->next()) {
				index = createIndex(_sqlQuery->record().value(0).toInt() - 1, fromIndex.column());
                break;
            }
            step *= stepIncrement;
            toPos = fromPos;
        }
    }
	_sqlQuery->finish();
    return index;
}
