#include <Windows.h>
#include <Winbase.h>
#include <gtest/gtest.h>
#include <QApplication>

#pragma comment( linker, "/subsystem:console" )
#define CONSOLE
void setGTestFilter() {
	testing::FLAGS_gtest_filter =
		//":*persistent_storage*"
		":*MessageFormatterTest*"
		//":*keys*"
		//":*bindTo*"
		;
}
#ifdef CONSOLE
int main()
{
	testing::InitGoogleTest(&__argc, __argv);
	setGTestFilter();
	RUN_ALL_TESTS();
}
#else
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	QApplication a(__argc, __argv);
	testing::InitGoogleTest(&__argc, __argv);
	setGTestFilter();
	RUN_ALL_TESTS();
}
#endif
