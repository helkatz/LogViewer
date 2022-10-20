#undef max
#undef min
#include <gui/mainwindow.h>
#include <gui/settings_dialog.h>

#include <utils/utils.h>

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


#include "LoggerHandler.h"
//#include <models/logstash/logstashmodel.h>
//#include "logfilemodel.h"

//Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

using namespace logger;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	static logger::Logger &l = logger::Logger::get("QtLogger");
	logger::Level level;
	switch (type) {
	case QtDebugMsg: 
		level = logger::Level::Debug;
		break;
	case QtInfoMsg:
		level = logger::Level::Info;
		break;
	case QtWarningMsg:
		level = logger::Level::Warning;
		break;
	case QtCriticalMsg:
		level = logger::Level::Critical;
		break;
	case QtFatalMsg:
		level = logger::Level::Error;
		break;
	}

	
	if (l.has_level(level)) {
		QByteArray localMsg = msg.toLocal8Bit();
		logger::LogStream(l, level, context.file, context.line, context.function) << localMsg.constData();
	}
}

void initLogging()
{
	qInstallMessageHandler(myMessageOutput);
	//logger::SqlLogMessageHandler& sqlLogHandler = *(new SqlLogMessageHandler);
	//if (sqlLogHandler.init())
	//	logger::Logger::register_message_handler(sqlLogHandler);

	logger::LogfileMessageHandler& logfileHandler = *(new LogfileMessageHandler);
	if (logfileHandler.init(appSettings().general().logFile()))
		logger::Logger::register_message_handler(logfileHandler);
/*
	logger::SyslogMessageHandler& syslogHandler = *(new SyslogMessageHandler);
	if (false && syslogHandler.init())
		logger::Logger::register_message_handler(syslogHandler);
*/
#ifdef _DEBUG
	logger::Logger::get()
		.set_level(logger::Logger::Level::Trace5)
		.set_delimiter(" ");
#else
	logger::Logger::get()
		.set_level(static_cast<logger::Logger::Level>(appSettings().general().logLevel()))
		.set_delimiter(" ");

#endif
}

#include <qstylefactory.h>
#include <gui\ColorPickerWidget.h>
void setTheme(QApplication& a)
{
	qApp->setStyle(QStyleFactory::create("Fusion"));

	QPalette darkPalette;
	darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::WindowText, Qt::white);
	darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
	darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
	darkPalette.setColor(QPalette::ToolTipText, Qt::white);
	darkPalette.setColor(QPalette::Text, Qt::white);
	darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::ButtonText, Qt::white);
	darkPalette.setColor(QPalette::BrightText, Qt::red);
	darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));

	darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	darkPalette.setColor(QPalette::HighlightedText, Qt::black);

	qApp->setPalette(darkPalette);
	
	qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
}

void testing()
{
	log_debug() << "test";
	qDebug() << appSettings()->childGroups();
	auto s = appSettings().logWindowTemplates("mytemplate");
	SettingsDialog dlg;
	dlg.exec();
	exit(0);
}
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	_putenv("QT_MESSAGE_PATTERN=\"[%{type}] %{appname} %{threadid} - %{message}\"");
	_putenv("QT_DEBUG_PLUGINS=1");
	//_putenv("QT_FATAL_WARNINGS=");
	qRegisterMetaTypeStreamOperators<ColorList>("ColorList");	
	initLogging();

	//QCoreApplication::addLibraryPath("c:/builds/logviewer/bin/debug/platforms");
    auto& a = *new QApplication(__argc, __argv);

	
	//Q_INIT_RESOURCE(default);
	//setTheme(a);

	QFile file("default.css");
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
	a.setStyleSheet(styleSheet);

	//LogUpdater::instance();
	//ObserverFile observeStyle;
	//observeStyle.createObserver(&a, "c:/builds/logviewer/default.css", [&a](const QString& id, int i) {
	//	QFile file("default.css");
	//	file.open(QFile::ReadOnly);
	//	QString styleSheet = QLatin1String(file.readAll());
	//	a.setStyleSheet(styleSheet);
	//});

	//QObject::connect(&observeStyle, &ObserverFile::observedObjectChanged, &a, [&a](const QString& id, int i) {
	//	QFile file("default.css");
	//	file.open(QFile::ReadOnly);
	//	QString styleSheet = QLatin1String(file.readAll());
	//	a.setStyleSheet(styleSheet);
	//});

#ifdef _DEBUG
	Logger::set_level(".*", Logger::Level::Trace);
	Logger::set_level(".*common.*", Logger::Level::Warning);
#endif	
	log_trace(0) << "startup";
	//testing();
 
	plugin_factory::Factory::LoadPlugins();

	//testing();
	//return 0;
	MainWindow::instance().show();	
	
    MainWindow::instance().readSettings();
    //QErrorMessage::qtHandler();
    return a.exec();
}
