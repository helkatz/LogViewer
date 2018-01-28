#pragma once
//#include "queryconditions.h"
#include "Properties.h"
#include <QAbstractTableModel>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <qdatetime.h>
#include <unordered_map>
typedef std::unordered_map<uint32_t, QSqlRecord> DataCache;

class Conditions: public Properties
{

public:
	PROPERTY(QString, connection)
    PROPERTY(QString, modelClass)
    PROPERTY(QString, queryString)
    PROPERTY(QDateTime, fromTime, QDateTime::fromTime_t(0))
    PROPERTY(QDateTime, toTime, QDateTime::fromTime_t(std::numeric_limits<time_t>::max()))
    PROPERTY(int, limit)
protected:
	
public:
    Conditions();
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};

class LogView;

class LogModel: public QSqlQueryModel
{
    Q_OBJECT
protected:
	struct CurrentRow
	{
		QSqlRecord record;
		quint64 row;
		CurrentRow() : row(-1) {}
		operator bool() { return row != -1; }
		void set(uint64_t row, const QSqlRecord& record)
		{
			this->row = row;
			this->record = record;
		}
		void reset()
		{
			row = -1;
		}
	};
	mutable CurrentRow _currentRow;

	QList<LogView *> _views;
    mutable DataCache _dataCache;
    int _rows;
    QSqlRecord _columnsInformation;
    Conditions _queryConditions;


    LogModel(QObject *parent);

    //static LogModel *createClass(const QString& className);
	virtual CurrentRow& loadData(const QModelIndex &index) const = 0;

public:
    void addView(LogView *view);

    void removeView(LogView *view);

public:
	virtual QVariant data(int row, int col, int role) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	virtual QString getTitle() const = 0;

    virtual void writeSettings(const QString& basePath) = 0;

    virtual void readSettings(const QString& basePath) = 0;

    virtual bool query(const Conditions& qc) = 0;

	virtual QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns, 
		const QString& search, bool regex, bool down) const = 0;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QStringList columns() const;

	QSqlRecord columnsInformation() const;

	virtual Conditions getQueryConditions() const;

	virtual void setQueryConditions(const Conditions& qc);


	virtual quint64 getFrontRow() const;

	virtual quint64 getBackRow() const;

	virtual int fetchMoreFrom(quint32 row, quint32 items, bool back);

	virtual int fetchToEnd();

	virtual int fetchToBegin();

	virtual int fetchMoreFromBegin(quint32 items);

	virtual int fetchMoreFromEnd(quint32 items);

public slots:
	virtual bool queryWithCondition(QString sqlFilter, int limit) = 0;
};

