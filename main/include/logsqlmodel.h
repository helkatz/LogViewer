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
#include "Utils/utils.h"

class LogView;
inline QString capitalize(const QString& s)
{
	return s[0].toUpper() + s.mid(1).toLower();
}

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

class LogModel : public QSqlQueryModel
{
    Q_OBJECT
protected:
	QList<LogView *> _views;
	mutable DataCache _dataCache;
	QSqlRecord _columnsInformation;
	Conditions _queryConditions;
    QString _lastFind;
    QString _queryFields;
    QString _query;
    QString _queryCount;
	int _rows;
    int _maxId;
    int _fromPos;
    QString _updateQuery;
    std::unique_ptr<utils::database::SqlQuery> _sqlQuery;

    QSqlField _autoincCol;

	virtual bool loadData(const QModelIndex &index) const;

    void createFilterTable(const QString& name, const QString& sql);

	SqlConditions qc() const
	{
		return _queryConditions;
	}

public:
	LogModel(QObject *parent);
	~LogModel();

	static void serialize();
	static LogModel *unserialize();

	void addView(LogView *view);

	void removeView(LogView *view);

	virtual Conditions getQueryConditions() const
	{
		return _queryConditions;
	}

	virtual void setQueryConditions(const Conditions& qc)
	{
		_queryConditions = qc;
	}

    void reportError(const QString& message);

	QVariant data(int row, int col, int role = Qt::DisplayRole) const;

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	virtual bool query(const Conditions& QueryOptions);

	virtual quint64 getFrontRow() const
	{
		return 0;
	}

	virtual quint64 getBackRow() const
	{
		return 0;
	}
	virtual int fetchMoreFrom(quint32 row, quint32 items, bool back)
	{
		Q_UNUSED(row);
		Q_UNUSED(items);
		Q_UNUSED(back);
		return 0;
	};
	virtual int fetchToEnd() { return 0; };
	virtual int fetchToBegin() { return 0; };
	virtual int fetchMoreFromBegin(quint32 items)
	{
		Q_UNUSED(items);
		return 0;
	};
	virtual int fetchMoreFromEnd(quint32 items)
	{
		Q_UNUSED(items);
		return 0;
	};
    virtual QString getTitle() const;

    virtual QModelIndexList match(const QModelIndex &start, int role,
                                  const QVariant &value, int hits = 1,
                                  Qt::MatchFlags flags =
                                  Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const;

    virtual QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns, const QString& search, bool regex, bool down) const;

	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QStringList columns() const;

	QSqlRecord getColumnsInformation() const
	{
		return _columnsInformation;
	}

    void queryRowsCount();

	virtual void updateRowCount(quint32 maxRows) 
	{
		Q_UNUSED(maxRows)
	};

    void writeSettings(const QString& basePath);

    void readSettings(const QString& basePath);

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

#endif // LOGSQLMODEL_H
