#include <utils/utils.h>
#include <logfilemodel.h>

#include <gtest/gtest.h>
using namespace logger;

class FilePluginTest : public LogFileModel, public ::testing::Test
{
public:
	FilePluginTest()
		: LogFileModel(nullptr)
	{}
};

TEST_F(FilePluginTest, random_read)
{	
	FileQueryParams conditions;
	conditions->bind(appSettings());
	conditions.fileName("c:/logs/logviewer.log");
	conditions.columnizer("default_columnizer");
	setQueryParams(conditions);
	query();

	for (int i = 1;; i++) {
		auto index = createIndex(std::rand() % rowCount(), 0);
		data(index, Qt::DisplayRole);
		if (i % 1000 == 0)
			qDebug() << i;
		if (i % 100000 == 0)
			query();
	}
}


