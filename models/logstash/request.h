#pragma once
#include <qjsondocument.h>
#include <QJsonValue>
#include <QPair>

#include <common/properties/Properties.h>
#include <string>
#include <map>
#include <vector>
namespace request {
	struct QueryRange
	{
		quint64 fromTime;
		quint64 toTime;
		quint32 from;
		quint32 size;
		quint64 index;
	};

	struct QueryConditions 
	{		
		QueryRange range;
		QString queryString;
		struct Aggs
		{
			uint64_t resolution;
		};
		boost::optional<Aggs> aggs;

	};

	struct IndexToTimeRange
	{
		class List : private std::vector<IndexToTimeRange>
		{
			using Parent = std::vector<IndexToTimeRange>;
			uint64_t total_ = 0;
			iterator findItemToUpdate(const IndexToTimeRange& needle);

		public:
			using Parent::size;
			using Parent::iterator;
			using Parent::back;
			using Parent::front;
			using Parent::end;
			using Parent::begin;

			/// merges the other list into this
			void merge(const List& other);

			/// rebuilds indices and resize items to max doc count 
			/// so list can shrink
			void rebuildIndex();

			/// counts up all docs 
			uint64_t countUp() const;

			/// returns the total document count
			uint64_t total() const;

			void push_back(const IndexToTimeRange& item);


			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & total_ & *this;
			}
		};
		quint64 index = 0;
		quint64 count = 0;
		quint64 fromTime = 0;
		quint64 toTime = 0;

		template<class Archive>
		void serialize(Archive & ar, const unsigned int /* file_version */) {
			ar & index & count & fromTime & toTime;
		}
	};
	struct Documents
	{
		struct Response
		{
			uint32_t took;	// time in ms
			bool timed_out;	// request timed out
			struct Aggs
			{
				quint64 total = 0;
				quint64 minTime = 0;
				quint64 maxTime = 0;
				IndexToTimeRange::List indexToTimeRange;
				template<class Archive>
				void serialize(Archive & ar, const unsigned int /* file_version */) {
					ar & total & minTime & maxTime & indexToTimeRange;
				}
				Aggs loadFromJson(const QJsonDocument& doc);
			} aggs;

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
				ar & took & timed_out & shards & aggs & hits;
			}
			Response loadFromJson(const QJsonDocument& doc);
		};

		using Criteria = QueryConditions;
		static QJsonDocument buildRequest(const Criteria& criteria);
		static constexpr const char *endpoint = "_search";
	};

	struct Mappings {
		struct Response
		{
			std::vector<std::string> properties;
			Response loadFromJson(const QJsonDocument& doc);
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & properties;
			}
		};

		using Criteria = QueryConditions;

		static QJsonDocument buildRequest(const Criteria& criteria);
		static constexpr const char *endpoint = "_mapping";
	};

	struct Histogram
	{
		struct Response
		{
			quint64 total = 0;
			quint64 minTime = 0;
			quint64 maxTime = 0;
			IndexToTimeRange::List indexToTimeRange;
			template<class Archive>
			void serialize(Archive & ar, const unsigned int /* file_version */) {
				ar & total & minTime & maxTime & indexToTimeRange;
			}
			Response loadFromJson(const QJsonDocument& doc);
		};

		struct Criteria {
			QueryConditions query;
			uint64_t resolution;			
		};

		static QJsonDocument buildRequest(const Criteria& criteria);
		static constexpr const char *endpoint = "_search";
	};

	class Request : public properties::Properties<Request>
	{
		using JsonMember = QPair<QString, QJsonValue>;

		prop_rw(QString, host);
		prop_rw(QString, index);
		prop_rw(QString, type);
	public:
		static QJsonDocument sendRequest(const QString& uri, const QString& data);

		template<typename T>
		bool fetch(const typename T::Criteria& criteria, typename T::Response& resp) const
		{
			auto jsonReq = T::buildRequest(criteria);

			QString uri = host;
			if (index.initialized() && index->length())
				uri += "/" + index;
			if (type.initialized() && type->length())
				uri += "/" + type;
			uri += "/";
			uri += T::endpoint, jsonReq.toJson();

			auto jsonDoc = sendRequest(uri, jsonReq.toJson());
			resp.loadFromJson(jsonDoc);
			return true;
		}
	};
}
