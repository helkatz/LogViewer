#include "queryconditions.h"

QString QueryConditions::get(const QString &name) const
{
    if(additional.find(name) != additional.end())
        return additional[name];
    return "";
}

void QueryConditions::set(const QString &name, const QString &value)
{
    additional[name] = value;
}

QueryConditions::QueryConditions():
    limit(10000)
{
}

void QueryConditions::writeSettings(const QString &basePath)
{
    Settings settings(basePath);
    settings.queryConditions().queryString(queryString);
    settings.queryConditions().limit(limit);
}

void QueryConditions::readSettings(const QString &basePath)
{
    Settings settings(basePath);
    queryString = settings.queryConditions().queryString();
    limit = settings.queryConditions().limit();
}
/*
SqlQueryConditions::SqlQueryConditions()
{
}

void SqlQueryConditions::writeSettings(const QString &basePath)
{
    Settings settings(basePath);
    settings.queryConditions().connectionName(connectionName);
    settings.queryConditions().tableName(tableName);
    QueryConditions::writeSettings(basePath);
}

void SqlQueryConditions::readSettings(const QString &basePath)
{
    Settings settings(basePath);
    connectionName = settings.queryConditions().connectionName();
    tableName = settings.queryConditions().tableName();
    QueryConditions::readSettings(basePath);
}

FileQueryConditions::FileQueryConditions()
{
}

void FileQueryConditions::writeSettings(const QString &basePath)
{
    Settings settings(basePath);
    settings.queryConditions().fileName(fileName);
    QueryConditions::writeSettings(basePath);
}

void FileQueryConditions::readSettings(const QString &basePath)
{
    Settings settings(basePath);
    fileName = settings.queryConditions().fileName();
    QueryConditions::readSettings(basePath);
}
*/
