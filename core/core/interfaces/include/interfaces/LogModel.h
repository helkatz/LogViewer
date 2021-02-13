#pragma once
#include <core/common.h>
#include <core/Properties.h>
#include <core/Settings.h>
#include <QAbstractTableModel>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QThread>
#include <QList>
#include <QSharedPointer>
#include <QTimer>
#include <qdatetime.h>
#include <unordered_map>
#include <chrono>
typedef std::unordered_map<uint32_t, QSqlRecord> DataCache;

using QueryParams = _settings::QueryParams;
//class CORE_API QueryParams: public Properties
//{
//	;
//public:
//	QString settingsPath() const;
//	PROPERTY(QueryParams, QString, connection)
//    PROPERTY(QueryParams, QString, modelClass)
//    PROPERTY(QueryParams, QString, queryString)
//    PROPERTY(QueryParams, quint64, fromTime, 0)
//    PROPERTY(QueryParams, quint64, toTime, std::numeric_limits<uint32_t>::max() * 1000ull)
//    PROPERTY(QueryParams, uint32_t, limit)
//
//protected:
//	
//public:
//	QueryParams(const QString& settingsPath);
//	QueryParams() = default;
//	virtual ~QueryParams() = default;
//	void writeSettings(_settings::QueryParams&);
//	void readSettings(_settings::QueryParams&);
//	template<typename Derived> 
//	Derived as()
//	{
//		Derived derived;
//		derived._hive = _hive;
//		derived.settingsPath_ = settingsPath_;
//		return derived;
//	}
//
//	template<typename Derived>
//	const Derived as() const
//	{
//		return const_cast<QueryParams*>(this)->as<Derived>();
//		Derived derived;
//		derived._hive = _hive;
//		return derived;
//	}
//};

class Observer : public QThread
{
	using EventHandler = std::function<void()>;
	EventHandler eventHandler_;
	QTimer checkChangesTimer_;
public:
	void install(std::chrono::milliseconds timer, EventHandler handler)
	{
		connect(&checkChangesTimer_, &QTimer::timeout, this, handler);
		checkChangesTimer_.setInterval(timer.count());
	}
	void run()
	{
		checkChangesTimer_.start();
	}
	void pause()
	{
		checkChangesTimer_.stop();
	}
};

class LogView;

class CORE_API LogModel: public QSqlQueryModel
{
	Q_OBJECT
protected:
	struct CurrentRow
	{
		QSqlRecord record;
		quint64 row;
		CurrentRow() : row(-1) {}
		operator bool() { return row != -1; }
		void set(quint64 row, const QSqlRecord& record)
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

	Observer observer_;
	QList<LogView *> _views;
    mutable DataCache _dataCache;
    int _rows;
    QSqlRecord _columnsInformation;
	QueryParams qp_;
	bool virtualRows_;

    LogModel(QObject *parent);

    //static LogModel *createClass(const QString& className);
	virtual CurrentRow& loadData(quint64 index) const = 0;

public:
    void addView(LogView *view);

    void removeView(LogView *view);

	bool virtualRows() const;
public:
	virtual QVariant data(int row, int col, int role) const;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	virtual QString getTitle() const = 0;

    virtual void writeSettings(_settings::QueryParams&);

    virtual void readSettings(_settings::QueryParams&);

    virtual bool query() = 0;

	virtual QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns, 
		const QString& search, bool regex, bool down) const = 0;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QStringList columns() const;

	QSqlRecord columnsInformation() const;

	virtual const _settings::QueryParams& getQueryParams() const;

	virtual void setQueryParams(const _settings::QueryParams& qc);

	//virtual Conditions getQueryConditions() const;

	//virtual void setQueryConditions(const Conditions& qc);

	virtual quint64 getFrontRow() const;

	virtual quint64 getBackRow() const;

	virtual int fetchMoreUpward(quint32 row, quint32 items);

	virtual int fetchMoreDownward(quint32 row, quint32 items);

	virtual int fetchToEnd();

	virtual int fetchToBegin();

	virtual int fetchMoreFromBegin(quint32 items);

	virtual int fetchMoreFromEnd(quint32 items);

	virtual void followTail(bool enabled);

public slots:
	virtual bool queryWithCondition(QString sqlFilter, int limit) = 0;
	virtual void processObserved() {};
};



