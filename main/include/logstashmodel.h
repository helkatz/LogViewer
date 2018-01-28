#pragma once
#include "logsqlmodel.h"
#include "logfile_parser.h"
//#include "Utils/utils.h"

//#include <QFile>
#include <QSharedPointer>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <mutex>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
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
	QString host() const 
	{
		return Settings().connections().logstash(connection()).host();
	}

	PROPERTY(QString, index, "logstash*")
	PROPERTY(QString, type, "syslog")
};

template<class Archive>
void serialize(Archive & ar, QString & s, const unsigned int version)
{
	std::string std_s = s.toStdString();
	ar & std_s;
	s = std_s;
}
struct Response
{
	uint32_t took;	// time in ms
	bool timed_out;	// request timed out
	struct Shards
	{
		uint32_t total;
		uint32_t successful;
		uint32_t skipped;
		uint32_t failed;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & total & successful & skipped & failed;
		}
	};
	Shards shards;
	struct Hits
	{
		uint32_t total;
		uint32_t max_score;
		struct Docs
		{
			std::string index;
			std::string config;
			std::string id;
			std::map<std::string, std::string> source;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & index & config & id;
			}
		};
		std::vector<Docs> docs;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & total & max_score & docs;
		}
	};
	Hits hits;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */) {
		ar & took & timed_out & shards & hits;
	}
	Response loadFromJson(const QJsonDocument& doc);
};

struct MappingsResponse
{
	std::vector<std::string> properties;
	MappingsResponse loadFromJson(const QJsonDocument& doc);
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */) {
		ar & properties;
	}
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

struct HistogramResponse
{
	struct IndexToTimeRange
	{
		using List = std::vector<IndexToTimeRange>;
		quint64 index = 0;
		quint64 count = 0;
		quint64 fromTime = 0;
		quint64 toTime = 0;
		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & index & count & fromTime & toTime;
		}
	};
	quint64 total;
	quint64 minTime;
	quint64 maxTime;
	IndexToTimeRange::List indexToTimeRange;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */) {
		ar & total & minTime & maxTime & indexToTimeRange;
	}
	HistogramResponse loadFromJson(const QJsonDocument& doc);
};

struct QueryRange
{
	quint64 fromTime;
	quint64 toTime;
	quint32 from;
	quint32 size;
	quint64 index;
};

class LogStashModelTest;
class LogStashModel : public LogModel
{
	Q_OBJECT
	friend class LogStashModelTest;
    
	template<typename Resp>
	void sendRequest(const QString& uri, const QString& data, Resp&) const;
	
	QJsonDocument sendRequest(const QString& uri, const QString& data) const;

	QueryRange getQueryRangeFromIndex(quint64 index);

	void loadQueryRangeList(
		const LogStashConditions& condtitions,
		uint64_t resolution = 1000, // 86400 * 365 * 1000, // about 1 year
		HistogramResponse::IndexToTimeRange::List::iterator *itemToUpdate = nullptr);

protected:

	CurrentRow& loadData(const QModelIndex &index) const;

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

    bool queryWithCondition(QString sqlFilter, int limit);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

private slots:
    void observedObjectChanged(const QString& id, const int maxId);
    void entriesCountChanged(quint32 newCount);

private:
	HistogramResponse::IndexToTimeRange::List _indexToTimeRange;
	uint32_t _maxDocsPerRange = 1000;	// maximum doc_count in a indexToTimeRange object
	uint32_t _loadDocsAtOnce = 200;		// how many docs should be loaded on data reload
	mutable std::mutex _mutex;

};

template<typename Resp>
inline void LogStashModel::sendRequest(const QString & uri, const QString & data, Resp& resp) const
{
	auto jsonDoc = sendRequest(uri, data);
	resp.loadFromJson(jsonDoc);
}
