#pragma once
#include "logsqlmodel.h"
#include "logfile_parser.h"
//#include "Utils/utils.h"

//#include <QFile>
#include <QSharedPointer>
//#include <qfilesystemwatcher.h>
//#include <qthread.h>
//#include <qfile.h>
//#include <qtextstream.h>
//#include <qregularexpression.h>

class LogStashConditions: public Conditions
{
public:
	LogStashConditions():
        Conditions()
    {
    }
	LogStashConditions(const Conditions& other):
        Conditions(other)
    {
    }
    PROPERTY(QString, fileName)
};

class LogStashModel : public LogModel
{
    Q_OBJECT
    QSharedPointer<Parser> _parser;
    mutable struct CurrentRow
    {
        QSqlRecord r;
        quint64 row;
        CurrentRow() : row(-1) {}
    } currentRow;

protected:
	LogStashConditions qc() const
        { return _queryConditions; }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	virtual quint64 getFrontRow() const;

	virtual quint64 getBackRow() const;

	virtual int fetchToEnd();

	virtual int fetchToBegin();

	virtual int fetchMoreFrom(quint32 row, quint32 items, bool back);

	virtual int fetchMoreFromBegin(quint32 items);

	virtual int fetchMoreFromEnd(quint32 items);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

public:
	LogStashModel(QObject *parent);

    bool query(const Conditions& qqueryConditions);

    bool queryWithCondition(QString sqlFilter, int limit);

    void writeSettings(const QString& basePath);

    void readSettings(const QString& basePath);

    QString getTitle() const;

    //void updateRowCount(quint32 maxRows);
    //QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const;

    QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns, const QString& search, bool regex, bool down) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

private slots:
    void observedObjectChanged(const QString& id, const int maxId);
    void entriesCountChanged(quint32 newCount);
};

