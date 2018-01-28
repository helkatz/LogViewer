#include "mainwindow.h"
#include "testing.h"
#include "logview.h"
#include "Utils/utils.h"
#include "Utils/LoggerSqlHandler.h"
#include "forms/QueryDialog.h"
#include "forms/columnizerwidget.h"
#include <QApplication>
#include <QTableView>
#include <QAbstractTableModel>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <qmdisubwindow.h>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QtPlugin>
#include <QException>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QDebug>
#include <qtimer.h>
#include <Windows.h>
#include <Winbase.h>
#include <QDir>
#include <QErrorMessage>
#include <qsqldatabase.h>
#include <mutex>
#include "logstashmodel.h"
#include "logfilemodel.h"

using namespace logger;

using namespace std;
template <typename Final>
class manip
{
public:
	void operator()(logger::LogStream &ios) const
	{
		(*static_cast<const Final *> (this))(ios);
	}
};

//template <class CharT>
class man1 : public manip< man1 >
{
public:

	man1()
	{
	}

	/**
	* Change the timezone and time format ios state;
	*/
	void operator()(logger::LogStream &ios) const
	{
		ios.set_delimiter("11");
	}
};
template <typename out_stream, typename manip_type>
out_stream &operator<<(out_stream &out, const manip<manip_type> &op)
{
	op(out);
	return out;
}
class myio : public std::stringstream
{
	int i;
public:
	//event_callback _Pfn;
	myio()
	{

	}

	myio(const myio& other)
	{
		i = other.i;		
	}
	myio& operator<<(myio& other)
	{
		return other;
	};
	myio& operator<<
		(myio& (*_Pfn)(myio& os))
	{
		return _Pfn(*this);
	};

	static myio& man1(myio& os)
	{
		os.i = 2;
		return os;
	};
	static myio& man2(myio& os)
	{
		os.i = 2;
		return os;
	};
};


int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	const QSettings::Format XmlFormat =
		QSettings::registerFormat("xml", readXmlFile, writeXmlFile);

	QSettings settings("test.ini", QSettings::Format::IniFormat);// QSettings::UserScope, "MySoft", "Star Runner");
	settings.setDefaultFormat(XmlFormat);
	Settings::setOrganisation("ACOM");
	Settings::setApplication("LogViewer");

	_putenv("QT_MESSAGE_PATTERN=\"[%{type}] %{appname} %{threadid} - %{message}\"");
	_putenv("QT_FATAL_WARNINGS=");
	qRegisterMetaTypeStreamOperators<ColorList>("ColorList");


	logger::SqlLogMessageHandler sqlLogHandler;
	if (sqlLogHandler.init())
		logger::Logger::register_message_handler(sqlLogHandler);

	logger::LogfileMessageHandler logfileHandler;
	if (logfileHandler.init(Settings().general().logFile()))
		logger::Logger::register_message_handler(logfileHandler);

	logger::SyslogMessageHandler syslogHandler;
	if (false && syslogHandler.init())
		logger::Logger::register_message_handler(syslogHandler);

#ifdef _DEBUG
	logger::Logger::get()
		.set_level(logger::Logger::Level::Trace5)
		.set_delimiter(" ");
#else
	logger::Logger::get()
		.set_level(static_cast<logger::Logger::Level>(Settings().general().logLevel()))
		.set_delimiter(" ");

#endif
	/*
	logger::Logger::set_level(".*", logger::Logger::Level::Trace1);
	logger::Logger::set_level("Parser.*", logger::Logger::Level::None);
	logger::Logger::set_level("Observer.*", logger::Logger::Level::Info);
	logger::Logger::set_level("LogView.*", logger::Logger::Level::Error);
	*/
	//log_trace(0) << "thread" << QThread::currentThreadId();
//2018-01-11 05:45:05.026442400 +0000|6|LogStashModel::loadQueryRangeList|testlog testlog test 000001
#if 0
	for (int i = 10000; i <= 19999; i++) {
		log_debug().set_delimiter("") << "testlog testlog testlog testlog testlog testlog " << i;
	}
	Sleep(100000);
#endif
	log_debug() << "startup logviewer";


    QApplication a(__argc, __argv);
	//new LogWindowTest();
	//LogStashModelTest().testGetData();
	//LogFileModelTest().testGetData();
	//WidgetTest();
	//WidgetTest::ColumnizerWidgetTest();
    MainWindow::instance().show();
    MainWindow::instance().readSettings();
    //QErrorMessage::qtHandler();
    return a.exec();
}
