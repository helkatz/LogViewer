#pragma once
//#include "queryconditions.h"
#include "Properties.h"
#include <QAbstractTableModel>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QThread>
#include <QSharedPointer>
#include <QTimer>
#include <qdatetime.h>
#include <unordered_map>
#include <chrono>
typedef std::unordered_map<uint32_t, QSqlRecord> DataCache;

class Conditions: public Properties
{
public:
	PROPERTY(Conditions, QString, connection)
    PROPERTY(Conditions, QString, modelClass)
    PROPERTY(Conditions, QString, queryString)
    PROPERTY(Conditions, uint64_t, fromTime, 0)
    PROPERTY(Conditions, uint64_t, toTime, std::numeric_limits<uint32_t>::max() * 1000ull)
    PROPERTY(Conditions, uint32_t, limit)
protected:
	
public:
    Conditions();
    virtual void writeSettings(const QString &basePath);
    virtual void readSettings(const QString &basePath);
};

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
class LogModel;
struct CreatorBase
{
	virtual QSharedPointer<LogModel> create() = 0;
};
template<typename T>
struct Creator: CreatorBase
{
	QSharedPointer<LogModel> create() override
	{
		return QSharedPointer<LogModel>{new T{ nullptr }};
	}
};
class LogModelFactory
{
	QMap<QString, QSharedPointer<CreatorBase>> registered_;
	static LogModelFactory* instance_;
public:
	template<typename T>
	static QSharedPointer<CreatorBase> Register(const QString& name)
	{
		if (instance_ == nullptr)
			instance_ = new LogModelFactory;
		QSharedPointer<CreatorBase> creator{ new Creator<T>() };
		instance_->registered_[name] = creator;
		return creator;
	}
	static QSharedPointer<LogModel> Create(const QString& name)
	{
		auto it = instance_->registered_.find(name);
		if (it == instance_->registered_.end())
			return nullptr;
		return it.value()->create();
	}
};

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

	Observer observer_;
	QList<LogView *> _views;
    mutable DataCache _dataCache;
    int _rows;
    QSqlRecord _columnsInformation;
    Conditions _queryConditions;


    LogModel(QObject *parent);

    //static LogModel *createClass(const QString& className);
	virtual CurrentRow& loadData(uint64_t index) const = 0;

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

	virtual int fetchMoreBackward(quint32 row, quint32 items);

	virtual int fetchMoreForward(quint32 row, quint32 items);

	virtual int fetchMoreFrom(quint32 row, quint32 items, bool back);

	virtual int fetchToEnd();

	virtual int fetchToBegin();

	virtual int fetchMoreFromBegin(quint32 items);

	virtual int fetchMoreFromEnd(quint32 items);

	virtual void followTail(bool enabled);

public slots:
	virtual bool queryWithCondition(QString sqlFilter, int limit) = 0;
	virtual void processObserved() {};
};

