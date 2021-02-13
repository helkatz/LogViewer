
#include <utils/utils.h>
#include <elasticmodel.h>

#include <gtest/gtest.h>

using namespace logger;

class ElasticModelTest : public ElasticModel, public ::testing::Test
{
public:
	ElasticModelTest()
		: ElasticModel(nullptr)
	{}
};

TEST_F(ElasticModelTest, test1)
{
	ElasticConditions conditions;
	conditions.connection("localhost");
	setQueryConditions(conditions);
	query(conditions);
	for(int i = 1;;i++) {
		//getQueryRangeFromIndex(std::rand() % model._rows);
		if(i%100000 == 0)
			query(conditions);
	}
}

TEST_F(ElasticModelTest, test2)
{
	ElasticConditions conditions;
	conditions.connection("localhost");
	setQueryConditions(conditions);
	query(conditions);
		
	for (int i = 1;; i++) {
		auto index = createIndex(std::rand() % rowCount(), 0);
		data(index, Qt::DisplayRole);
		if (i % 1000 == 0)
			qDebug() << i;
		if (i % 100000 == 0)
			query(conditions);
	}
}
