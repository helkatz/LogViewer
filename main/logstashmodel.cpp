#include <boost/asio.hpp>
#include "logupdater.h"
#include "logview.h"
#include "logstashmodel.h"

#include "common.h"
#include "Utils/utils.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpressionMatch>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QUrlQuery>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QThread>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <string>
#include <deque>
#include <iostream>
#include <Poco/JSON/JSON.h>
#include <boost/optional.hpp>
#include <qhttpmultipart.h>
#include <qdatastream.h>
#include <mutex>
#include <WinSock2.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <string>

//#define qDebug() if(0) QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO).debug()

#include <fstream>
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
	class SearchRequest
	{
		QJsonDocument _doc;
		QJsonObject _root;
		SearchRequest()
		{
			_doc.setObject(_root);
		}
		SearchRequest& index(const QStringList&);
		SearchRequest& fields(const QStringList& fields)
		{
			QJsonArray arr;
			arr.fromStringList(fields);
			_root.insert("_source", arr);
			return *this;
		}
		SearchRequest& from(qint64 fromPos)
		{
			_root.insert("from", fromPos); return *this;
		}
		SearchRequest& size(qint32 size)
		{
			_root.insert("size", size); return *this;
		}

		QJsonDocument& toJson()
		{
			return _doc;
		}
	};
}

LogStashModel::LogStashModel(QObject *parent):
    LogModel(parent)
{
	_maxDocsPerRange = 1000;
	_loadDocsAtOnce = 100;
}
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
	void run()
	{
		log_indent()
		//mutex.lock();
		static int index = 0;
		std::string response;
		//Serializer ser(QString("Request%1.dat").arg(++index));
		//bool gotresp = ser.get(response);
		if (true) {
			QNetworkAccessManager manager;
			QUrl url(uri);
			log_debug() << "request" << url.toString() << data;
			QNetworkRequest request(url);
			request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
			QUrlQuery params;
			QByteArray ba;
			QNetworkReply *repl;
			url.setQuery(params);
			if (data.length()) {
				ba.append(data);
				repl = manager.post(request, ba);
			}
			else
				repl = manager.get(request);
			QEventLoop loop;
			connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
			loop.exec();
			//const_cast<LogStashModel*>(this)->blockSignals(false);
			auto byteArr = repl->readAll();
			if (byteArr.length() == 0 || repl->error() != QNetworkReply::NoError) {
				log_error() << "reply from server" << repl->errorString();				
				//throw std::exception("network error");
			}
			//if (gotresp == false) {
			response = byteArr.data();
			
		}
		this->response = response.c_str();
		finished = true;
	}
};
QJsonDocument LogStashModel::sendRequest(const QString& uri, const QString& data) const
{
	try {
		Requester req(qc().host() + uri, data);
		while (req.finished == false)
			QThread::msleep(10);
		QJsonParseError err;
		QJsonDocument doc = QJsonDocument::fromJson(req.response.toStdString().c_str(), &err);
		return doc;
	}
	catch (std::exception& e) {
		return QJsonDocument{};
	}
}

LogModel::CurrentRow& LogStashModel::loadData(const QModelIndex &index) const
{	
	log_func_entry_leave();
	log_debug() << "rowIndex" << index.row() << "not found in dataCache{"<<_dataCache.size()<<"}";
__LTryAgain:
	try {
		QueryRange range = const_cast<LogStashModel*>(this)->getQueryRangeFromIndex(index.row());

		QJsonDocument json;
		json.setObject(QJsonObject{
			{ "query", QJsonObject{
				//{ "match_all", QJsonObject{} },
				{ "range", QJsonObject{
					{"@timestamp", QJsonObject{
						{"gte", std::to_string(range.fromTime).c_str()},
						{"lte" , std::to_string(range.toTime).c_str()}
					}}
				}}
			}},
			{"from", QJsonValue::fromVariant(range.from)},
			{"size", QJsonValue::fromVariant(range.size)}
		});

		Response resp;
		sendRequest("/*/syslog/_search", json.toJson(), resp);
		
		uint32_t cacheIndex = range.index;
		_currentRow.reset();
		if (resp.hits.docs.size()) {
			QSqlRecord r = _columnsInformation;
			for (auto& doc : resp.hits.docs) {
				for (auto& field : doc.source) {
					auto pos = r.indexOf(field.first.c_str());
					QSqlField f;
					f.setName(field.first.c_str());
					f.setValue(field.second.c_str());
					r.replace(pos, f);
				}
				if (cacheIndex == index.row()) {
					_currentRow.set(index.row(), r);
				}
				_dataCache[cacheIndex++] = r;
				
			}
		}
		return _currentRow;
	}
	catch (...) {
		log_debug() << "failed to load";
		QThread::sleep(1);
		goto __LTryAgain;
	}
}

QueryRange LogStashModel::getQueryRangeFromIndex(quint64 index)
{
	log_func_entry_leave();
	//log_trace(0) << "rowIndex" << index;
	boost::optional<uint64_t> resolution;

	while (true) {
		auto item = std::find_if(
			_indexToTimeRange.begin(), _indexToTimeRange.end(),
			[index](const HistogramResponse::IndexToTimeRange& el)
		{
			return index >= el.index && index < el.index + el.count;
		});		
		bool refetch = false;
		if (item != _indexToTimeRange.end()) {
			// fromIndex ist the index in the item range and this should not be to great
			// otherwise query time costs
			// so when it exeed then we have to reload index for this part with more resolution
			quint32 fromIndex = index - item->index;
			
			if (fromIndex < 1000) {
				size_t preLoadSize = _loadDocsAtOnce / 2;
				while(preLoadSize-- > 0
					&& fromIndex > 0
					&& _dataCache.find(index) == _dataCache.end())
				{
					index--;
					fromIndex = index - item->index;
				}
				
				return QueryRange{ 
					item->fromTime, item->toTime, 
					fromIndex, _loadDocsAtOnce,  
					index
				};
			}
			refetch = true;
			log_trace(0) << "index" <<index<<" found in indexToTimeRange refetch needed"<< item->index << "count"<<item->count;
		} else
			log_trace(0) << "index"<<index<<" not found int indexToTimeRange";
		LogStashConditions conditions = qc();

		// when there is a matched item then its reolution is to low
		// so we reload only that item with an higher resolution and merge it into the current list
		uint64_t res = resolution ? *resolution : 0;
		if (!resolution) {
			resolution = refetch ? (item->toTime - item->fromTime) * 1000 / item->count : 86400 * 365 * 1000;
		}
		else
			*resolution /= 2;
		if (*resolution == 0)
			*resolution = 1;

		if (refetch) {
			QDateTime dt;
			dt.setMSecsSinceEpoch(item->fromTime);
			conditions.fromTime(dt);
			dt.setMSecsSinceEpoch(item->toTime);
			conditions.toTime(dt);
			//log_trace(0) << "updateItem " << item- _indexToTimeRange.begin() << item->index << _indexToTimeRange.front().index;
			loadQueryRangeList(conditions, *resolution, &item);
		}
		else {
			loadQueryRangeList(conditions, *resolution);
		}
	}
}

void LogStashModel::loadQueryRangeList(
	const LogStashConditions & condtitions, uint64_t resolution, 
	HistogramResponse::IndexToTimeRange::List::iterator *itemToUpdate)
{

	auto count = [](const HistogramResponse::IndexToTimeRange::List& list) {
		uint32_t total = 0;
		for (auto& e : list) {
			total += e.count;
		}
		return total;
	};
	log_func_entry_leave();
	log_trace(0) << "resolution" << resolution << "updateItem=" << (itemToUpdate != nullptr);
	QJsonDocument json;
	json.setObject(QJsonObject{
		{ "query", QJsonObject{
			//{ "match_all", QJsonObject{} },
			{ "range", QJsonObject{
				{"@timestamp", QJsonObject{
					{"gte", std::to_string(condtitions.fromTime().toMSecsSinceEpoch()).c_str()},
					{"lte" , std::to_string(condtitions.toTime().toMSecsSinceEpoch()).c_str()}
				}}
			}}
		}},
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
					{"interval", QString("%1ms").arg(resolution)}
				}}
			}}
		}}
	});

	HistogramResponse respHistogram;
	sendRequest("/*/syslog/_search", json.toJson(), respHistogram);

	if (itemToUpdate) {
		auto& item = **itemToUpdate;
		auto pos = (*itemToUpdate) - _indexToTimeRange.begin();
		if (itemToUpdate)
			log_trace(0) << "updateItem after request"
				<< pos << item.index << item.count << "\n"
				<< _indexToTimeRange.size() << count(_indexToTimeRange) << "\n"
				<< respHistogram.indexToTimeRange.size() << count(respHistogram.indexToTimeRange) << "\n";

		_indexToTimeRange.insert(*itemToUpdate + 1, 
			respHistogram.indexToTimeRange.begin(),
			respHistogram.indexToTimeRange.end());

		_indexToTimeRange.erase(*itemToUpdate);
	}
	else {
		_rows = respHistogram.total;
		_indexToTimeRange = respHistogram.indexToTimeRange;
	}

	if (_indexToTimeRange.size()) {

		if (_rows != count(_indexToTimeRange))
			throw std::exception("invalid timeRange count _rows != count(_indexToTimeRange)");

		auto current = _indexToTimeRange.begin();
		std::for_each(_indexToTimeRange.begin() + 1, _indexToTimeRange.end(), 
			[&current, this](HistogramResponse::IndexToTimeRange e) {
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
		_indexToTimeRange.erase(current + 1, _indexToTimeRange.end());
	}
	log_trace(0) << "done size="
		<< _indexToTimeRange.size() << "count" << count(_indexToTimeRange);
}

bool LogStashModel::query(const Conditions &queryConditions)
{	
	log_func_entry_leave();

	_dataCache.clear();
	_columnsInformation.clear();

	uint32_t col = 0;
	MappingsResponse mappings;
	sendRequest("/*/syslog/_mapping", "", mappings);
	foreach(const auto& name, mappings.properties) {
		QSqlField f;
		f.setName(name.c_str());
		_columnsInformation.append(f);
		setHeaderData(col++, Qt::Horizontal, tr("%1").arg(capitalize(name.c_str())));
	}
	loadQueryRangeList(queryConditions);
	emit layoutChanged();
    return true;
}

bool LogStashModel::queryWithCondition(QString sqlFilter, int limit)
{
    qc().queryString(sqlFilter);
    qc().limit(limit);
    return query(qc());
}

void LogStashModel::writeSettings(const QString &basePath)
{
    qc().writeSettings(basePath);
}

void LogStashModel::readSettings(const QString &basePath)
{
    qc().readSettings(basePath);
}

QString LogStashModel::getTitle() const
{
    return qc().host();
}

void LogStashModel::observedObjectChanged(const QString& id, const int maxId)
{
}

void LogStashModel::entriesCountChanged(quint32 newCount)
{
}

QModelIndex LogStashModel::find(const QModelIndex& fromIndex, const QStringList & columns, const QString& search, bool regex, bool down) const
{
	Q_UNUSED(regex);
	Q_UNUSED(columns);
    qDebug() << "find fromPos:" << fromIndex.row() << "dir:" << down << "where:" << search;
    QModelIndex index = fromIndex;
	return index;
}

QModelIndex LogStashModel::index(int row, int column, const QModelIndex &parent) const
{
    auto index = QSqlQueryModel::index(row, column, parent);
    if (index.isValid())
        return index;
    if (row >= rowCount()) {
        //auto delta = row - rowCount();
        //const_cast<LogStashModel *>(this)->updateRowCount(row);
        //emit const_cast<LogStashModel *>(this)->layoutChanged();
    }
    return QSqlQueryModel::index(rowCount() - 1, column, parent);

}

Response Response::loadFromJson(const QJsonDocument& doc)
{
	static int index = 0;
	Serializer ser(QString("Response%1.dat").arg(++index));
	if (ser.get(*this))
		return *this;
	log_func_entry_leave();
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
			
			for(auto& ohits : ohits.value("hits").toArray()) {
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

MappingsResponse MappingsResponse::loadFromJson(const QJsonDocument& doc)
{
	static int index = 0;
	Serializer ser(QString("MappingsResponse%1.dat").arg(++index));
	if (ser.get(*this))
		return *this;
	for (auto keyIndex : doc.object().keys()) {
		auto oIndexes = doc.object().value(keyIndex);
		for (auto keyType : oIndexes.toObject().value("mappings").toObject().keys()) {
			auto oTypes = oIndexes.toObject().value("mappings").toObject().value(keyType);
			for (auto keyProperty : oTypes.toObject().value("properties").toObject().keys()) {
				if(std::find_if(
					properties.begin(), properties.end(), 
					[&keyProperty](const std::string& v) {
						return v == keyProperty.toStdString();
					}) == properties.end())
						properties.push_back(keyProperty.toStdString());
			}
			
		}
	}
	ser.write(*this);
	return *this;
}

HistogramResponse HistogramResponse::loadFromJson(const QJsonDocument& doc)
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
	uint64_t previndex = 0;
	uint32_t sumDocs = 0;
	uint64_t fromTime;
	
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

