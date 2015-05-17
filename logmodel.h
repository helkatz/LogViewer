#pragma once
#include "queryconditions.h"
#include "Properties.h"
#include <QAbstractTableModel>
#include <QSqlTableModel>
#include <QSqlRecord>
typedef std::map<int, QSqlRecord> DataCache;

class Conditions: public Properties
{

public:
    PROPERTY(QString, modelClass)
    PROPERTY(QString, queryString)
    PROPERTY(QDateTime, fromTime)
    PROPERTY(QDateTime, toTime)
    PROPERTY(int, limit)
protected:

public:
    Conditions();
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};

class LogModel: public QSqlQueryModel
{
    Q_OBJECT
protected:
    QObjectList _views;
    mutable DataCache _dataCache;
    int _rows;
    QSqlRecord _columnsInformation;
    Conditions _queryConditions;
    LogModel(QObject *parent);
    static LogModel *createClass(const QString& className);

    virtual bool loadData(const QModelIndex &index) const = 0;
public:
    void addView(QObject *view);

    void removeView(QObject *view);

public:

    static void serialize();
    static LogModel *unserialize();
    static LogModel *create(Conditions &queryConditions);
    //static LogModel *create(const QString& settingsPath);

    virtual Conditions getQueryConditions() const
        { return _queryConditions; }

    virtual void setQueryConditions(const Conditions& qc)
        { _queryConditions = qc; }

    QVariant data(int row, int col, int role = Qt::DisplayRole) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    virtual void writeSettings(const QString& basePath) = 0;

    virtual void readSettings(const QString& basePath) = 0;

    virtual bool query(const Conditions& qc) = 0;

    virtual QModelIndex find(const QModelIndex& fromIndex, QString where, bool down) const = 0;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QStringList columns() const;

    QSqlRecord getColumnsInformation() const {
        return _columnsInformation;
    }

    virtual QString getTitle() = 0;
};
/*
inline QVariant LogModel::data(int row, int col, int role) const
{
    QModelIndex index = createIndex(row, col);
    return QAbstractTableModel::data(index, role);
}*/
