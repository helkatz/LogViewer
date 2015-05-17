#ifndef QUERYCONDITIONS_H
#define QUERYCONDITIONS_H
#include "settings.h"
#include <QString>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSharedPointer>

class QueryConditions
{
    QMap<QString, QString> additional;
public:
    typedef QSharedPointer<QueryConditions> Ptr;
    QString modelClass;
    QString queryString;
    QDateTime fromTime;
    QDateTime toTime;
    QDateTime logDate;
    QString orderBy;
    int limit;

    QString get(const QString& name) const;
    void set(const QString& name, const QString& value);
    operator Ptr() const { return Ptr(new QueryConditions(*this)); }
protected:

public:
    QueryConditions();
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};



class SqlQueryConditions: public QueryConditions
{
public:
    QString connectionName;
    QString tableName;
    QSqlDatabase database;

    SqlQueryConditions();
    operator Ptr() const { return Ptr(new SqlQueryConditions(*this)); }
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};

class FileQueryConditions: public QueryConditions
{
protected:

public:
    QString fileName;
    FileQueryConditions();
    operator Ptr() const { return Ptr(new FileQueryConditions(*this)); }
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};

#endif // QUERYCONDITIONS_H
