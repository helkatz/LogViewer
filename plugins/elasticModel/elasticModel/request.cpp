#include "request.h"
#include "elasticmodel.h"

#include <core/common.h>
#include <utils/utils.h>

#include <QEventLoop>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QUrlQuery>
#include <QMessageBox>
#include <QThread>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPBasicCredentials.h>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <mutex>
#include <string>
#include <fstream>
#include <istream>
class Serializer
{
	QString fileName;
public:
	Serializer(const QString& key) :
		fileName(key)
	{
	}

	template<typename T>
	bool get(T& o)
	{
		return false;
		std::ifstream fs(fileName.toStdString());
		if (fs.is_open() == false)
			return false;
		boost::archive::text_iarchive a(fs);
		a >> o;
		return true;
	}

	template<typename T>
	bool write(const T& o)
	{
		return false;
		std::ofstream fs(fileName.toStdString());
		boost::archive::text_oarchive a(fs);
		a << o;
		return true;
	}
};
#define qxDebug() log_debug() 
namespace {
	template<class Archive>
	void serialize(Archive & ar, QString & s, const unsigned int version)
	{
		std::string std_s = s.toStdString();
		ar & std_s;
		s = std_s;
	}

	using JsonMember = QPair<QString, QJsonValue>;
	auto jsonQuery(const QJsonArray& arr)
	{
		return JsonMember{
			"query", QJsonObject{
				{"bool", QJsonObject{
					{ "must", arr }
				}}
			}
		};
	}
	auto jsonQueryObject(const QString& queryString) {
		if (queryString.length()) {
			return QJsonObject{
				{ "query_string", QJsonObject{
					{ "query", queryString }
				}}
			};
		}
		return QJsonObject{};
	}
	auto jsonRange(const request::QueryRange& range) {
		return QJsonObject{
			{ "range", QJsonObject{
				{ "@timestamp", QJsonObject{
					{ "gte", std::to_string(range.fromTime).c_str() },
					{ "lte" , std::to_string(range.toTime).c_str() }
				}}
			}} 
		};
	}

	template<typename Resp>
	void sendRequest(const QString & uri, const QString & data, Resp& resp)
	{
		auto jsonDoc = sendRequest(uri, data);
		resp.loadFromJson(jsonDoc);
	}
}

#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/StreamCopier.h>
std::mutex mutex;

class Requester: public QThread
{
public:
	QString uri;
	QString data;
	QString response;
	bool finished = false;
public:
	Requester(const QString& uri, const QString& data) :
		uri(uri),
		data(data)
	{
		moveToThread(this);
		start();
	}
	static bool sendRequest(const QString& suri, const QString& data, QString& response)
	{
		Poco::URI uri(suri.toStdString().c_str());
		std::string path(uri.getPathAndQuery());
		if (path.empty()) path = "/";
		Poco::Net::HTTPClientSession remote(
			uri.getHost(), uri.getPort()
		);

		std::string method = "POST";
		if (data.length() == 0)
			method = "GET";
		Poco::Net::HTTPRequest httpReq(method, path);

		httpReq.setContentType("application/json");
		httpReq.setContentLength(data.length());

		Poco::Net::HTTPBasicCredentials auth;
		auth.setPassword("Spock360##a");
		auth.setUsername("h.katz");
		auth.authenticate(httpReq);
		//httpReq.setCredentials(auth);
		//httpReq.setCredentials("h.katz", "Spock360##a");
		std::string jdata = data.toStdString();
		qDebug() << QUrl(suri) << jdata.c_str();
		auto &reqBody = remote.sendRequest(httpReq);
		reqBody << data.toStdString();
		reqBody.flush();

		Poco::Net::HTTPResponse httpResponse;

		auto &responseBody = remote.receiveResponse(httpResponse);
		if (httpResponse.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
			response = httpResponse.getReason().c_str();
			log_error() << "reply from server" << response;
			return false;
		}
		std::string stdResponse;
		Poco::StreamCopier::copyToString(responseBody, stdResponse);
		response = stdResponse.c_str();

		return true;
	}
};

namespace request {
	QJsonDocument Request::sendRequest(const QString& uri, const QString& data)
	{
		try {
#if 1
			QString response;
			Requester::sendRequest(uri, data, response);

			QJsonParseError err;
			QJsonDocument doc = QJsonDocument::fromJson(response.toStdString().c_str(), &err);
			return doc;
#else
			Requester req(uri, data);
			while (req.finished == false)
				QThread::msleep(10);

			QJsonParseError err;
			QJsonDocument doc = QJsonDocument::fromJson(req.response.toStdString().c_str(), &err);
			return doc;
#endif
		}
		catch (std::exception& e) {
			return QJsonDocument{};
		}
	}

	QJsonDocument Documents::buildRequest(const Documents::Criteria& criteria)
	{
		QJsonDocument json;
		
		auto root = QJsonObject();
		
		auto query = QJsonArray{};
		query.append(jsonRange(criteria.range));
		if (criteria.queryString.length())
			query.append(jsonQueryObject(criteria.queryString));

		root.insert("query", QJsonObject{
			{ "bool", QJsonObject{
				{ "must", query }
			}}
		});

		root.insert("from", QJsonValue::fromVariant(criteria.range.from));
		root.insert("size", QJsonValue::fromVariant(criteria.range.size));

		if (criteria.aggs) {
			root.insert(
				"aggs", QJsonObject{
					{"max_time", QJsonObject{
						{ "max", QJsonObject{
							{ "field", "@timestamp" }
						}}
					}},
					{"min_time", QJsonObject{
						{ "min", QJsonObject{
							{ "field", "@timestamp" }
						}}
					}},
					{"histogram", QJsonObject{
						{"date_histogram", QJsonObject{
							{"min_doc_count", 1},
							{"field", "@timestamp"},
							{"interval", QString("%1ms").arg(criteria.aggs->resolution)}
						}}
					}}
				}
			);
		}
		json.setObject(root);
		return json;
	}

	QJsonDocument Histogram::buildRequest(const Histogram::Criteria& criteria)
	{
		QJsonDocument json;

		auto query = QJsonArray{};
		query.append(jsonRange(criteria.query.range));
		if (criteria.query.queryString.length())
			query.append(jsonQueryObject(criteria.query.queryString));


		json.setObject(QJsonObject{
			jsonQuery(query),
			{"size", 0},
			{"aggs", QJsonObject{
				{"max_time", QJsonObject{
					{ "max", QJsonObject{
						{ "field", "@timestamp" }
					}}
				}},
				{"min_time", QJsonObject{
					{ "min", QJsonObject{
						{ "field", "@timestamp" }
					}}
				}},
				{"histogram", QJsonObject{
					{"date_histogram", QJsonObject{
						{"min_doc_count", 1},
						{"field", "@timestamp"},
						{"interval", QString("%1ms").arg(criteria.resolution)}
					}}
				}}
			}}
		});
		return json;
	}

	QJsonDocument Mappings::buildRequest(const Criteria& criteria)
	{
		return QJsonDocument{};
	}

	Documents::Response::Aggs Documents::Response::Aggs::loadFromJson(const QJsonDocument& doc)
	{
		static int index = 0;
		Serializer ser(QString("HistogramResponse%1.dat").arg(++index));
		if (ser.get(*this))
			return *this;
		auto& root = doc.object();
		total = root["hits"].toObject()["total"].toVariant().toULongLong();

		auto& aggr = root.value("aggregations").toObject();
		maxTime = aggr["max_time"].toObject()["value"].toVariant().toULongLong();
		minTime = aggr["min_time"].toObject()["value"].toVariant().toULongLong();
		auto& buckets = aggr["histogram"].toObject()["buckets"].toArray();
		quint64 previndex = 0;
		uint32_t sumDocs = 0;

		for (auto& e : buckets) {
			//qDebug() << e;
			IndexToTimeRange tr;
			tr.toTime = maxTime;
			tr.fromTime = e.toObject()["key"].toVariant().toULongLong();
			tr.count = e.toObject()["doc_count"].toVariant().toULongLong();


			if (indexToTimeRange.size())
				indexToTimeRange.back().toTime = tr.fromTime - 1;
			indexToTimeRange.push_back(tr);
		}
		if (indexToTimeRange.size())
			indexToTimeRange.front().fromTime = minTime;
		ser.write(*this);
		return *this;
	}

	Documents::Response Documents::Response::loadFromJson(const QJsonDocument& doc)
	{
		static int index = 0;
		Serializer ser(QString("Response%1.dat").arg(++index));
		if (ser.get(*this))
			return *this;
		log_func_entry_leave();
		
		aggs.loadFromJson(doc);

		auto root = doc.object();
		took = root.value("took").toInt();

		timed_out = root.value("timed_out").toInt();
		{
			auto& o = root.value("_shards").toObject();
			shards.total = o.value("total").toInt();
			shards.successful = o.value("successful").toInt();
			shards.skipped = o.value("skipped").toInt();
			shards.failed = o.value("failed").toInt();
		}
		{
			auto& ohits = root.value("hits").toObject();
			hits.total = ohits.value("total").toInt();
			{

				for (auto& ohits : ohits.value("hits").toArray()) {
					auto& ohit = ohits.toObject();
					Hits::Docs doc;
					doc.index = ohit.value("_index").toString().toStdString();
					doc.id = ohit.value("_id").toString().toStdString();
					doc.config = ohit.value("_config").toString().toStdString();
					for (auto source : ohit.value("_source").toObject().keys()) {
						auto value = ohit.value("_source").toObject().value(source).toString();;
						doc.source[source.toStdString()] = value.toStdString();
					}
					hits.docs.push_back(doc);
				}
			}

		}
		ser.write(*this);
		return *this;
	}

	Mappings::Response Mappings::Response::loadFromJson(const QJsonDocument& doc)
	{
		static int index = 0;
		Serializer ser(QString("MappingsResponse%1.dat").arg(++index));
		if (ser.get(*this))
			return *this;
		for (auto keyIndex : doc.object().keys()) {
			auto oMappings = doc.object().value(keyIndex).toObject().value("mappings");
			for (auto keyProperty : oMappings.toObject().value("properties").toObject().keys()) {
				if (std::find_if(
					properties.begin(), properties.end(),
					[&keyProperty](const std::string& v) {
					return v == keyProperty.toStdString();
				}) == properties.end())
					properties.push_back(keyProperty.toStdString());
			}
		}
		ser.write(*this);
		return *this;
	}

	Histogram::Response Histogram::Response::loadFromJson(const QJsonDocument& doc)
	{
		static int index = 0;
		Serializer ser(QString("HistogramResponse%1.dat").arg(++index));
		if (ser.get(*this))
			return *this;
		auto& root = doc.object();
		total = root["hits"].toObject()["total"].toVariant().toULongLong();

		auto& aggr = root.value("aggregations").toObject();
		maxTime = aggr["max_time"].toObject()["value"].toVariant().toULongLong();
		minTime = aggr["min_time"].toObject()["value"].toVariant().toULongLong();
		auto& buckets = aggr["histogram"].toObject()["buckets"].toArray();
		quint64 previndex = 0;
		uint32_t sumDocs = 0;

		for (auto& e : buckets) {
			//qDebug() << e;
			IndexToTimeRange tr;
			tr.toTime = maxTime;
			tr.fromTime = e.toObject()["key"].toVariant().toULongLong();
			tr.count = e.toObject()["doc_count"].toVariant().toULongLong();


			if (indexToTimeRange.size())
				indexToTimeRange.back().toTime = tr.fromTime - 1;
			indexToTimeRange.push_back(tr);
		}
		if (indexToTimeRange.size())
			indexToTimeRange.front().fromTime = minTime;
		ser.write(*this);
		return *this;
	}

	IndexToTimeRange::List::iterator 
		IndexToTimeRange::List::findItemToUpdate(
			const request::IndexToTimeRange& needle) {

		auto it = std::lower_bound(
			begin(), end(), needle, [](const auto &a, const auto &b)
		{
			return a.fromTime < b.fromTime;
		});
		if (it != end()
			&& it->fromTime > needle.fromTime) {
			it--;
		}
		if (it == end()
			&& size() > 0
			&& needle.fromTime <= back().toTime) {
			it = end() - 1;
		}
		return it;
	};

	void IndexToTimeRange::List::merge(const List & other)
	{
		if (other.size() == 0)
			return;

		auto fromItem = findItemToUpdate(other.front());

		if (fromItem == end()) {
			insert(end(), other.begin(), other.end());
			total_ += other.countUp();
		}
		else {
			auto pos = fromItem - begin();
			erase(fromItem);
			insert(begin() + pos, other.begin(), other.end());
		}
		rebuildIndex();		
	}

	void IndexToTimeRange::List::rebuildIndex()
	{
		auto current = begin();
		std::for_each(begin() + 1, end(),
			[&](request::IndexToTimeRange e)
		{
			if ((current->count + e.count) < 1000) {
				current->count += e.count;
				current->toTime = e.toTime;
			}
			else {
				e.index = current->index + current->count;
				current++;
				*current = e;
			}
		});
		erase(current + 1, end());

	}

	quint64 IndexToTimeRange::List::countUp() const
	{
		quint64 total = 0;
		for (auto& e : *this) {
			total += e.count;
		}
		return total;
	}

	quint64 IndexToTimeRange::List::total() const
	{
		return total_;
	}

	void IndexToTimeRange::List::push_back(const IndexToTimeRange& item)
	{
		total_ += item.count;
		Parent::push_back(item);
	}
}