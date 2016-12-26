#include "mainwindow.h"
#include "logview.h"
#include "Utils/utils.h"
#include "Utils/LoggerSqlHandler.h"

#include "forms/QueryDialog.h"

#include <QApplication>
#include <QTableView>
#include <QAbstractTableModel>
#include <QSqlTableModel>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QtPlugin>
#include <QException>
#include <QDateTime>
#include <QProcessEnvironment>
#include <QDebug>
#include <Windows.h>
#include <Winbase.h>
#include <QDir>
#include <QErrorMessage>
#include <qsqldatabase.h>
#include <mutex>

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
template<typename T>
std::string get(T)
{
	return "ti";
}

template<>
std::string get(const char *ti)
{
	return "char *;";
}

class classfunc
{
public: 
	std::string name()
	{
		return __FUNCDNAME__;
	}
	std::string sig()
	{
		return __FUNCSIG__;
	}
};

//int _stdcall WinMain(int argc, char *argv[])
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
{
	Settings::setOrganisation("ACOM");
	Settings::setApplication("LogViewer");
	_putenv("QT_MESSAGE_PATTERN=\"[%{type}] %{appname} %{threadid} - %{message}\"");
	_putenv("QT_FATAL_WARNINGS=");
	qRegisterMetaTypeStreamOperators<ColorList>("ColorList");

	logger::SqlLogMessageHandler logHandler;
	if (logHandler.init())
		logger::Logger::register_message_handler(logHandler);
	logger::Logger::get()
		.set_level(static_cast<logger::Logger::Level>(Settings().general().logLevel()))
		.set_delimiter(" ");
	logger::Logger::set_level("Parser.*", logger::Logger::Level::Trace10);
	logger::Logger::set_level("Observer.*", logger::Logger::Level::Info);

    QApplication a(__argc, __argv);
    MainWindow::instance().show();
    MainWindow::instance().readSettings();
    //QErrorMessage::qtHandler();
    return a.exec();
}
