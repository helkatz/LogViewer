#pragma once
#include "elasticModel.h"
#include "request.h"

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

struct ElasticModel::Private
{
	Private(ElasticModel *owner)
		: owner_(*owner)
	{}

	request::QueryRange getQueryRangeFromIndex(quint64 index);

	quint64 loadQueryRangeList(
		const ElasticConditions& condtitions,
		quint64 resolution = 1000);

	static void increaseResolution(const request::IndexToTimeRange& item
		, boost::optional<uint64_t>& resolution);

	request::Request request_;
	request::Documents::Response::Aggs histogram_;
	uint32_t maxDocsPerRange_ = 1000;	// maximum doc_count in a indexToTimeRange object
	uint32_t loadDocsAtOnce_ = 200;		// how many docs should be loaded on data reload
	mutable std::mutex mutex_;
	ElasticConditions conditions_;
	ElasticModel& owner_;
};

void ElasticModel::Private::increaseResolution(const request::IndexToTimeRange& item
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

quint64 ElasticModel::Private::loadQueryRangeList(
	const ElasticConditions & condtitions, quint64 resolution)
{
	log_func_entry_leave();
	//log_trace(0) << "resolution" << resolution << "updateItem=" << (itemToUpdate != nullptr);

	request::QueryRange range{};
	range.fromTime = condtitions.fromTime();
	range.toTime = condtitions.toTime();

	request::Documents::Criteria criteria{ range, condtitions.queryString(),{ { resolution } } };

	request::Documents::Response respHistogram;
	request_.fetch<request::Documents>(criteria, respHistogram);
	//request::Histogram::fetch(criteria, respHistogram);

	histogram_.indexToTimeRange.merge(respHistogram.aggs.indexToTimeRange);

	if (histogram_.indexToTimeRange.size()) {
		if (histogram_.indexToTimeRange.total() != histogram_.indexToTimeRange.countUp())
			throw std::exception("invalid timeRange count _rows != count(histogram.indexToTimeRange)");
	}
	log_trace(0) << "done size="
		<< histogram_.indexToTimeRange.size() << "count"
		<< histogram_.indexToTimeRange.total();

	return histogram_.total;
}

request::QueryRange ElasticModel::Private::getQueryRangeFromIndex(quint64 index)
{
	log_func_entry_leave();
	//log_trace(0) << "rowIndex" << index;
	boost::optional<quint64> resolution;

	while (true) {
		if (histogram_.indexToTimeRange.size()) {
			auto item = histogram_.indexToTimeRange.end() - 1;
			// when index exeeds then we have to update the end
			if (index >= item->index + item->count) {
				ElasticConditions conditions = conditions_
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

			if (fromIndex < maxDocsPerRange_) {
				// load items before the index when not already in dataCache
				// for this we determine a new fromIndex where we want to fetch
				size_t preLoadSize = loadDocsAtOnce_ / 2;
				while (preLoadSize-- > 0
					&& fromIndex > 0
					&& owner_._dataCache.find(index) == owner_._dataCache.end())
				{
					index--;
					fromIndex = index - item->index;
				}

				return request::QueryRange{
					item->fromTime, item->toTime,
					fromIndex, loadDocsAtOnce_,
					index
				};
			}
			log_trace(0) << "index" << index << " found in indexToTimeRange refetch needed" 
				<< item->index << "count" << item->count;

			// when we come here with an item then its reolution is to low
			// means it would have more docs as _maxDocsPerRange
			// so we reload only that item with an higher resolution and merge it into the current list
			increaseResolution(*item, resolution);

			ElasticConditions conditions = conditions_
				.fromTime(item->fromTime)
				.toTime(item->toTime);

			log_trace(0) << "updateItem " << item - histogram_.indexToTimeRange.begin() 
				<< item->index << histogram_.indexToTimeRange.front().index;
			loadQueryRangeList(conditions, *resolution);

		}
		else {
			log_trace(0) << "index" << index << " not found int indexToTimeRange";
			resolution = 86400 * 365 * 1000;
			loadQueryRangeList(conditions_, *resolution);
		}
	}
	return request::QueryRange{};
}