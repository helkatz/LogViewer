#include "mainwindow.h"
#include "testing.h"
#include "logview.h"
#include <logupdater.h>
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
	logger::SqlLogMessageHandler& sqlLogHandler = *(new SqlLogMessageHandler);
	if (sqlLogHandler.init())
		logger::Logger::register_message_handler(sqlLogHandler);

	logger::LogfileMessageHandler& logfileHandler = *(new LogfileMessageHandler);
	if (true && logfileHandler.init(Settings().general().logFile()))
		logger::Logger::register_message_handler(logfileHandler);

	logger::SyslogMessageHandler& syslogHandler = *(new SyslogMessageHandler);
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

#include <qtoolbutton.h>
class QMyWidget : public QWidget
{
public:
	QMyWidget(QWidget * parent) : QWidget(parent)
	{
		auto btn = new QToolButton(this);
		auto layout = new QGridLayout;
		layout->addWidget(btn);
		setLayout(layout);
	}
};
#include <forms/TextColorizerWidget.h>
#include <forms/ColorPickerWidget.h>
#include <forms/rowlayoutwidget.h>
#include <TextColorizer.h>
#include <qtableview.h>
#include <qdatastream.h>
struct Part {
	using List = QList<Part>;
	QString text;
	int index;
	TextColorize *colorize;
};
QDebug& operator << (QDebug& os, const Part& part)
{
	os << part.text << ":" << part.index;// (part.colorize != nullptr);
	return os;
}
void playground(QApplication& a)
{
	std::vector<int> l1 = { 0,1,3,6,9,11 };
	auto fromItem = std::upper_bound(l1.begin(), l1.end(), 2
		, [](const auto &a, const auto &b)
	{
		return a > b;
	});

	return;
	TextColorize::List _coloredTextParts;
	auto match = [&_coloredTextParts](const QString& text, Part::List& parts)
	{
		QRegularExpression::PatternOptions options;
		/*if (tc.caseSensitive == false)
		options |= QRegularExpression::PatternOption::CaseInsensitiveOption;
		*/
		QString rePattern;
		foreach(TextColorize ctp, _coloredTextParts) {
			QString pattern;
			if (ctp.caseSensitive == false)
				pattern += "(?i)";
			pattern += ctp.text;
			pattern.prepend("(").append(")");
			if(ctp.wordOnly)
				pattern.prepend("(?:^|[^\\w])").append("(?:[^\\w]|$)");
			rePattern += "|" + pattern;
		}
		rePattern = rePattern.mid(1);
		QRegularExpression re(rePattern, options);

		QRegularExpressionMatchIterator it = re.globalMatch(text);

		int nonMatchedStart = 0;
		bool hasMatches = it.hasNext();
		while (it.hasNext()) {
			QRegularExpressionMatch match = it.next();

			int matchedGroup = match.lastCapturedIndex();
			while (match.capturedTexts().at(matchedGroup).length() && --matchedGroup);

			qDebug()
				<< match.capturedTexts() << " - "
				<< match.capturedView() << " - "
				<< match.hasPartialMatch() << " - "
				<< matchedGroup;
			int nonMatechedEnd = match.capturedStart(0);
			


			int nonMatchedLength = nonMatechedEnd - nonMatchedStart;
			//auto& ct = _coloredTextParts[match.lastCapturedIndex() - 1];
			auto& ct = _coloredTextParts[0];
			if (nonMatchedLength)
				;// parts.push_back({ text.mid(nonMatchedStart, nonMatchedLength), nullptr });
			parts.push_back({ text.mid(match.capturedStart(0), match.capturedLength(0)), match.lastCapturedIndex() });
			nonMatchedStart = match.capturedEnd(0);
		}
		if (nonMatchedStart < text.length())
			parts.push_back({ text.mid(nonMatchedStart), false });
		return hasMatches;
	};
	{
		Part::List parts;
		QString text = "this is testing and test and Test and error 367 so on is";
		
		TextColorize tc{};
		tc.caseSensitive = true;
		tc.text = "test";
		_coloredTextParts.push_back(tc);
		tc.caseSensitive = false;
		tc.text = "test";
		_coloredTextParts.push_back(tc);
		tc.caseSensitive = false;
		tc.wordOnly = true;
		tc.text = "is";
		_coloredTextParts.push_back(tc);
		tc.caseSensitive = false;	tc.wordOnly = true; 	tc.text = "ing";
		_coloredTextParts.push_back(tc);
		tc.caseSensitive = false;	tc.wordOnly = false; 	tc.text = "(error.*?\\d+)";
		_coloredTextParts.push_back(tc);

		match(text, parts);
		qDebug() << parts;
		return;
	}
	RowStyle rs;
	QTableWidget tv;
	tv.setRowCount(3);
	tv.setColumnCount(4);
	RowLayoutWidget rlw(nullptr, rs, tv.model()->index(0, 0));
	rlw.show();
	a.exec();
	exit(0);
	/*ColorPickerWidget colorPicker(nullptr);
	colorPicker.show();
	a.exec();*/
	QMainWindow wnd;
	QMyWidget myWidget(nullptr);
	TextColorizerWidget tc(nullptr);
	TextColorize::List list;
	//TextColorize c{"text"};
	//list.push_back(TextColorize{ "text1", QFont(), false, true, true, true, QColor(0xff00ff), QColor() });
	//list.push_back(TextColorize{ "text2", QFont(), false, true, true, true, QColor(0xff00ff), QColor() });
	while (true) {
		tc.initialize(list);
		tc.resize(500, tc.height());

		/*if(tc.exec() == QDialog::Rejected)
			break;*/
		list = tc.colorizeList();
	}
	list = tc.colorizeList();
	exit(0);
	return;
}
int testing()
{	
	//new LogWindowTest();
	LogFileModelTest().testGetData();
	//LogStashModelTest().testGetData();
	//LogFileModelTest().testGetData();

	//WidgetTest::ColumnizerWidgetTest();
	return 0;
}
#include <qstylefactory.h>
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
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	const QSettings::Format XmlFormat =
		QSettings::registerFormat("xml", readXmlFile, writeXmlFile);

	QSettings settings("test.ini", QSettings::Format::IniFormat);
	settings.setDefaultFormat(XmlFormat);
	Settings::setOrganisation("ACOM");
	Settings::setApplication("LogViewer");

	//_putenv("QT_MESSAGE_PATTERN=\"[%{type}] %{appname} %{threadid} - %{message}\"");
	//_putenv("QT_FATAL_WARNINGS=");
	qRegisterMetaTypeStreamOperators<ColorList>("ColorList");	
	initLogging();

	//QCoreApplication::addLibraryPath("c:/builds/logviewer/bin/debug/platforms");
    QApplication a(__argc, __argv);

	//Q_INIT_RESOURCE(default);
	//setTheme(a);

	QFile file("default.css");
	file.open(QFile::ReadOnly);
	QString styleSheet = QLatin1String(file.readAll());
	a.setStyleSheet(styleSheet);

	LogUpdater::instance();
	ObserverFile observeStyle;
	observeStyle.createObserver(&a, "c:/builds/logviewer/default.css", [&a](const QString& id, int i) {
		QFile file("default.css");
		file.open(QFile::ReadOnly);
		QString styleSheet = QLatin1String(file.readAll());
		a.setStyleSheet(styleSheet);
	});

	QObject::connect(&observeStyle, &ObserverFile::observedObjectChanged, &a, [&a](const QString& id, int i) {
		QFile file("default.css");
		file.open(QFile::ReadOnly);
		QString styleSheet = QLatin1String(file.readAll());
		a.setStyleSheet(styleSheet);
	});
	playground(a);
	testing();
#ifdef _DEBUG
	Logger::set_level(".*", Logger::Level::Trace);
	Logger::set_level(".*common.*", Logger::Level::Warning);
#endif	
	log_trace(0) << "startup";


    MainWindow::instance().show();
    MainWindow::instance().readSettings();
    //QErrorMessage::qtHandler();
    return a.exec();
}
