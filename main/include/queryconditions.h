#pragma once

#include "settings.h"
#include <QString>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSharedPointer>

class QueryConditions
{
    QMap<QString, QString> additional;
public:
    QString modelClass;
    QString queryString;
    QDateTime fromTime;
    QDateTime toTime;
    QDateTime logDate;
    QString orderBy;
    int limit;

    QString get(const QString& name) const;
    void set(const QString& name, const QString& value);
protected:

public:
	QueryConditions();
	QueryConditions(const QueryConditions&);
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};



class SqlQueryOptions : public QueryConditions
{
public:
    QString connectionName;
    QString tableName;
    QSqlDatabase database;

	SqlQueryOptions();
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};

class FileQueryOptions : public QueryConditions
{
public:

    QString fileName;
	FileQueryOptions();
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};

