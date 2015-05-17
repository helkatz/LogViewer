#ifndef LOGSQLMODEL_H
#define LOGSQLMODEL_H
#include <QSqlQueryModel>
#include <QString>
#include <QDateTime>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QList>
#include <vector>
#include <map>
#include <QModelIndex>
#include <QSqlField>
#include <QObjectList>
#include "queryconditions.h"
#include "logmodel.h"
class ColumnInformation
{
public:
    QSqlRecord _record;
    int _colorize;
public:
    QSqlRecord& record()
    {
        return _record;
    }
    void setColorize(int colorize) { _colorize = colorize; }
    int colorize() const { return _colorize; }

};

class SqlConditions: public Conditions
{
public:
    SqlConditions():
        Conditions()
    {
    }
    SqlConditions(const Conditions& other):
        Conditions(other)
    {
    }
    PROPERTY(QString, database)
    PROPERTY(QString, tableName)
    PROPERTY(QString, connectionName)
};

class LogSqlModel: public LogModel
{
    Q_OBJECT
protected:


    QString _lastFind;

    QString _queryFields;
    QString _query;
    QString _queryCount;
    int _maxId;
    int _fromPos;
    QString _updateQuery;
    QSqlQuery *_sqlQuery;

    //QSqlRecord _columnsInformation;
    QSqlField _autoincCol;
    //SqlQueryConditions _queryConditions;

    bool loadData(const QModelIndex &index) const;

    void createFilterTable(const QString& name, const QString& sql);
public:
    SqlConditions qc() const
        { return _queryConditions; }
    /*QSqlRecord getColumnsInformation() const {
        return _columnsInformation;
    }*/
    /*
    SqlQueryConditions& getQueryConditions()
        { return _queryConditions; }

    void setQueryConditions(const QueryConditions& qc)
        { _queryConditions = qc; }
    */
    void reportError(const QString& message);

    LogSqlModel(QObject *parent);

    ~LogSqlModel();

    //QVariant data(int row, int col, int role = Qt::DisplayRole) const;

    //QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool query(const Conditions& queryConditions);

    QString getTitle();

    virtual QModelIndexList match(const QModelIndex &start, int role,
                                  const QVariant &value, int hits = 1,
                                  Qt::MatchFlags flags =
                                  Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const;

    QModelIndex find(const QModelIndex& fromIndex, QString where, bool down) const;

    //int getColorizeColumn(int index) { return _coloredColumns.find(index) != _coloredColumns.end() ? _coloredColumns[index] : 0; }

    void queryRowsCount();

    void writeSettings(const QString& basePath);

    void readSettings(const QString& basePath);

signals:
    void newQuery(const QString& connectionName, const QString& table);
    void dataChanged();
public slots:

private slots:

    void dataChanged(const QString& connectionName, const QString& table, int maxId);
    void updateQuery(int maxId);
public slots:
    void queryWithCondition(QString sqlFilter, int limit);
};

#endif // LOGSQLMODEL_H
