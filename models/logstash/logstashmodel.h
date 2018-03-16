#pragma once
#include "request.h"

#include <models/logmodel.h>
#include <utils/utils.h>
#include <settings.h>

//#include <QSharedPointer>
//#include <qjsondocument.h>
//#include <qjsonobject.h>


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
	QString host() const 
	{
		return Settings().connections().logstash(connection()).host();
	}

	PROPERTY(LogStashConditions, QString, index, "")
	PROPERTY(LogStashConditions, QString, type, "")
};

struct Resolution
{
public:
	enum Enum {
		Second,
		Minute,
		Hour,
		Day,
		Month,
		Year,
	};

	Resolution(Enum value) :
		value(value)
	{};

	QString toString()
	{
		if (value == Enum::Year) return "year";
		if (value == Enum::Month) return "month";
		if (value == Enum::Day) return "day";
		if (value == Enum::Hour) return "hour";
		if (value == Enum::Minute) return "minute";
		if (value == Enum::Second) return "second";
	}

	void dec() {
		value = static_cast<Enum>(value > Second ? value - 1 : Second);
	}
private:
	Enum value;
};


class LogStashModelTest;
class LogStashModel : public LogModel
{
	Q_OBJECT
	friend class LogStashModelTest;
    
	template<typename Resp>
	void sendRequest(const QString& uri, const QString& data, Resp&) const;
	
	QJsonDocument sendRequest(const QString& uri, const QString& data) const;

	request::QueryRange getQueryRangeFromIndex(quint64 index);

	void loadIndexToTimeRange(
		const LogStashConditions& condtitions,
		uint64_t resolution);

	uint64_t loadQueryRangeList(
		const LogStashConditions& condtitions,
		uint64_t resolution = 1000);

protected:

	CurrentRow& loadData(uint64_t index) const override;

	LogStashConditions qc() const
        { return _queryConditions; }

    //QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
public:
	LogStashModel(QObject *parent);

	virtual QString getTitle() const override;

	void writeSettings(const QString& basePath) override;

	void readSettings(const QString& basePath) override;

	bool query(const Conditions& QueryOptions) override;

	virtual QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns,
		const QString& search, bool regex, bool down) const override;

    bool queryWithCondition(QString sqlFilter, int limit) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

	void processObserved() override;

private slots:
    void observedObjectChanged(const QString& id, const int maxId);
    void entriesCountChanged(quint32 newCount);

private:
	request::Request request_;
	request::Documents::Response::Aggs histogram_;
	//request::Histogram::Response::IndexToTimeRange::List _indexToTimeRange;
	uint32_t _maxDocsPerRange = 1000;	// maximum doc_count in a indexToTimeRange object
	uint32_t _loadDocsAtOnce = 200;		// how many docs should be loaded on data reload
	mutable std::mutex _mutex;

};

