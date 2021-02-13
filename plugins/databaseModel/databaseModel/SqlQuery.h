#pragma once

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

#include "Settings.h"
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

