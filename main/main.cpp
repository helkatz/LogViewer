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

//Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

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


void initLogging()
{
	return;
	logger::SqlLogMessageHandler sqlLogHandler;
	if (sqlLogHandler.init())
		logger::Logger::register_message_handler(sqlLogHandler);

	logger::LogfileMessageHandler logfileHandler;
	if (false && logfileHandler.init(Settings().general().logFile()))
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
}

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

	//QCoreApplication::addLibraryPath("c:/builds/logviewer/bin/debug/platforms");
    QApplication a(__argc, __argv);
	
	initLogging();

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
