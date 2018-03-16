#pragma once
#include "queryconditions.h"
#include <models/logmodel.h>
#include <utils/utils.h>

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

class LogView;

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
    PROPERTY(SqlConditions, QString, database)
    PROPERTY(SqlConditions, QString, tableName)
    PROPERTY(SqlConditions, QString, connectionName)
};

class LogDatabaseModel : public LogModel
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
    std::unique_ptr<utils::database::SqlQuery> _sqlQuery;

    QSqlField _autoincCol;

	SqlConditions qc() const { return _queryConditions;	}

	CurrentRow& loadData(uint64_t index) const override;
public:
	LogDatabaseModel(QObject *parent);

	~LogDatabaseModel();

    void reportError(const QString& message);

	virtual QString getTitle() const override;

	void writeSettings(const QString& basePath) override;

	void readSettings(const QString& basePath) override;

	bool query(const Conditions& QueryOptions) override;

    virtual QModelIndexList match(const QModelIndex &start, int role,
                                  const QVariant &value, int hits = 1,
                                  Qt::MatchFlags flags =
                                  Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const;

    QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns, 
		const QString& search, bool regex, bool down) const override;


signals:
    void newQuery(const QString& connectionName, const QString& table);
    void dataChanged();
	void setModifiedPos(quint32 row);
public slots:

private slots:

	void observedObjectChanged(const QString& id, const int maxId);
    void updateQuery(int maxId);
public slots:
    virtual bool queryWithCondition(QString sqlFilter, int limit);
};
