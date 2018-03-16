#include <models/logstash/logstashmodel.h>

#include <logupdater.h>
//#include <logview.h>
#include <common.h>
#include <Utils/utils.h>

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

#include <fstream>
namespace {
	auto reg = LogModelFactory::Register<LogStashModel>("LogStashModel");

	void increaseResolution(const request::IndexToTimeRange& item
		, boost::optional<uint64_t>& resolution)
	{
		uint64_t res = resolution ? *resolution : 0;
		if (!resolution) {
			resolution = (item.toTime - item.fromTime) * 1000 / item.count;
		}
		else
			*resolution /= 2;
		if (*resolution == 0)
			*resolution = 1;
	}
}

LogStashModel::LogStashModel(QObject *parent):
    LogModel(parent)
{
	_maxDocsPerRange = 1000;
	_loadDocsAtOnce = 100;
}

LogModel::CurrentRow& LogStashModel::loadData(uint64_t index) const
{	
	log_func_entry_leave();
	log_debug() << "rowIndex" << index << "not found in dataCache{"<<_dataCache.size()<<"}";
__LTryAgain:
	try {
		request::QueryRange range = const_cast<LogStashModel*>(this)->getQueryRangeFromIndex(index);

		request::Documents::Criteria criteria{ range, qc().queryString() };
		request::Documents::Response resp;

		request_.fetch<request::Documents>(criteria, resp);
		
		uint32_t cacheIndex = range.index;
		_currentRow.reset();
		if (resp.hits.docs.size()) {
			for (auto& doc : resp.hits.docs) {
				QSqlRecord r = _columnsInformation;
				for (auto& field : doc.source) {
					auto pos = r.indexOf(field.first.c_str());
					QSqlField f;
					f.setName(field.first.c_str());
					f.setValue(field.second.c_str());
					r.replace(pos, f);
				}
				if (cacheIndex == index) {
					_currentRow.set(index, r);
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

request::QueryRange LogStashModel::getQueryRangeFromIndex(quint64 index)
{
	log_func_entry_leave();
	//log_trace(0) << "rowIndex" << index;
	boost::optional<uint64_t> resolution;

	while (true) {
		if (histogram_.indexToTimeRange.size()) {
			auto item = histogram_.indexToTimeRange.end() - 1;
			// when index exeeds then we have to update the end
			if (index >= item->index + item->count) {
				LogStashConditions conditions = qc()
					.fromTime(item->toTime + 1)
					.toTime(std::numeric_limits<time_t>::max());

				resolution = 86400 * 365 * 1000;

				if (loadQueryRangeList(conditions, *resolution) == 0)
					return request::QueryRange{};
			}
		}
		// check if we have a item range for the required index
		auto item = std::find_if(
			histogram_.indexToTimeRange.begin(), histogram_.indexToTimeRange.end(),
			[index](const request::IndexToTimeRange& el)
		{
			return index >= el.index && index < el.index + el.count;
		});

		if (item != histogram_.indexToTimeRange.end()) {
			// fromIndex is the index in the item range and this should not be to great
			// otherwise query time costs
			// so when it exeed then we have to reload index for this part with more resolution
			quint32 fromIndex = index - item->index;

			if (fromIndex < _maxDocsPerRange) {
				// load items before the index when not already in dataCache
				// for this we determine a new fromIndex where we want to fetch
				size_t preLoadSize = _loadDocsAtOnce / 2;
				while (preLoadSize-- > 0
					&& fromIndex > 0
					&& _dataCache.find(index) == _dataCache.end())
				{
					index--;
					fromIndex = index - item->index;
				}

				return request::QueryRange{
					item->fromTime, item->toTime,
					fromIndex, _loadDocsAtOnce,
					index
				};
			}
			log_trace(0) << "index" << index << " found in indexToTimeRange refetch needed" << item->index << "count" << item->count;

			// when we come here with an item then its reolution is to low
			// means it would have more docs as _maxDocsPerRange
			// so we reload only that item with an higher resolution and merge it into the current list
			increaseResolution(*item, resolution);

			LogStashConditions conditions = qc()
				.fromTime(item->fromTime)
				.toTime(item->toTime);

			log_trace(0) << "updateItem " << item - histogram_.indexToTimeRange.begin() << item->index << histogram_.indexToTimeRange.front().index;
			loadQueryRangeList(conditions, *resolution);
			
		}
		else {
			log_trace(0) << "index" << index << " not found int indexToTimeRange";
			resolution = 86400 * 365 * 1000;
			loadQueryRangeList(qc(), *resolution);
		}
	}
	return request::QueryRange{};
}

uint64_t LogStashModel::loadQueryRangeList(
	const LogStashConditions & condtitions, uint64_t resolution)
{
	log_func_entry_leave();
	//log_trace(0) << "resolution" << resolution << "updateItem=" << (itemToUpdate != nullptr);
	
	request::QueryRange range{};
	range.fromTime = condtitions.fromTime();
	range.toTime = condtitions.toTime();

	request::Documents::Criteria criteria{ range, qc().queryString(), {{resolution}} };

	request::Documents::Response respHistogram;
	request_.fetch<request::Documents>(criteria, respHistogram);
	//request::Histogram::fetch(criteria, respHistogram);
	
	histogram_.indexToTimeRange.merge(respHistogram.aggs.indexToTimeRange);

	_rows = histogram_.indexToTimeRange.total();
	if (histogram_.indexToTimeRange.size()) {
		if (_rows != histogram_.indexToTimeRange.countUp())
			throw std::exception("invalid timeRange count _rows != count(histogram_.indexToTimeRange)");
	}
	log_trace(0) << "done size="
		<< histogram_.indexToTimeRange.size() << "count" 
		<< histogram_.indexToTimeRange.total();
	
	return histogram_.total;
}

bool LogStashModel::query(const Conditions &queryConditions)
{	
	log_func_entry_leave();

	_dataCache.clear();
	_columnsInformation.clear();

	uint32_t col = 0;
	request_.host = qc().host();
	request_.index = qc().index();
	request_.type = qc().type();
	request::Mappings::Response mappings;
	request::QueryConditions criteria;
	request_.fetch<request::Mappings>(criteria, mappings);

	foreach(const auto& name, mappings.properties) {
		QSqlField f;
		f.setName(name.c_str());
		_columnsInformation.append(f);
		setHeaderData(col++, Qt::Horizontal, tr("%1").arg(capitalize(name.c_str())));
	}
	loadQueryRangeList(queryConditions);

	observer_.install(std::chrono::milliseconds{ 1000 }, [this] {
		if(loadData(_rows))
			emit layoutChanged();
	});
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

void LogStashModel::processObserved()
{
	loadData(_rows);
}

