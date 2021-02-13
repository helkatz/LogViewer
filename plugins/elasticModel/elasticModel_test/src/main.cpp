
//#include <gtest/gtest.h>
//#include <common/logger.h>
#include <iostream>
#include <array>
#include <new>

using namespace std;


int main(int argc, char **argv)
{
#if 0
	::testing::InitGoogleTest(&argc, argv);
	::testing::GTEST_FLAG(filter) = 
		"xxFileTest*"
		":xxCronTrigger*"
		":xxPerformanceTest*"
		":xxPropertiesDataTest*"
		":xxPropertiesTest_2_0*"
		":MiscTest*";
	//::testing::GTEST_FLAG(filter) = "CronTrigger*";
	logger::Logger::set_level(".*", logger::Logger::Level::Trace10);
	logger::handlers::ConsoleMessageHandler consoleHandler;
	logger::Logger::register_message_handler(consoleHandler);
	return RUN_ALL_TESTS();
#endif
}
