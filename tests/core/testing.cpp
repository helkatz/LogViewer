#if 0
#include "testing.h"
#include <gui/logview/LogView.h>
#include <gui/QueryDialog.h>
#include <gui/columnizerwidget.h>
#include <gui/rowlayoutwidget.h>
#include <gui/connectionswidget.h>

#include <utils/utils.h>
#include <utils/LoggerSqlHandler.h>

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

#if 0
using namespace logger;

class GuiPlay
{
	void playRowStyleWidget
	{
		RowStyle rs;
		QTableWidget tv;
		tv.setRowCount(3);
		tv.setColumnCount(4);
		RowLayoutWidget rlw(nullptr, rs, tv.model()->index(0, 0));
		rlw.show();
		a.exec();
	}
};
void WidgetTest::ConnectionsWidgetTest()
{
	ConnectionsWidget w;
	w.resize(500, w.height());
	w.exec();
	exit(0);
}
void WidgetTest::RowLayoutWidgetTest()
{
/*	RowStyle style;
	RowLayoutWidget w(nullptr, style, QModelIndex());
	w.resize(1000, w.height());
	w.exec();
	exit(0);*/
}
void WidgetTest::ColumnizerWidgetTest()
{
	ColumnizerWidget w;
	w.resize(1000, w.height());
	w.exec();
	exit(0);
}


void LogWindowTest::run()
{
	while (!MainWindow::_instance)
		Sleep(500);
	auto& area = *MainWindow::instance().mdiArea;
	while (area.subWindowList().size() == 0)
		Sleep(1000);
	QMdiSubWindow *window = area.subWindowList().at(0);
		
	//area.setActiveSubWindow(window);
	LogWindow *logView = qobject_cast<LogWindow *>(window->widget());
	QTimer scrollTimer;
//	log_trace(0) << "thread" << currentThreadId();
	connect(this, &LogWindowTest::scrolltable, logView->mainView_.data(), [logView](QModelIndex index) {
//		log_trace(0) << "thread" << currentThreadId();
		logView->mainView_->scrollTo(index);
	});
			
	connect(&scrollTimer, &QTimer::timeout, this, [this, &logView]() {
		auto pos = rand() % 40000;
		auto model = logView->model();
		//QModelIndex index = model->currentIndex();
		auto index = logView->model()->index(pos, 0);
		//model->setCurrentIndex(index);
		emit scrolltable(index);
		//logView->_logView->scrollTo(index);
	});
	scrollTimer.start(100);
	exec();
}

LogWindowTest::LogWindowTest()
{
	moveToThread(this);
	start();
}

#endif
#endif