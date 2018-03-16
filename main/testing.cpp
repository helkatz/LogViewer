#include "testing.h"
#include "mainwindow.h"
#include "logview.h"
#include "Utils/utils.h"
#include "Utils/LoggerSqlHandler.h"
#include "forms/QueryDialog.h"
#include "forms/columnizerwidget.h"
#include <forms/rowlayoutwidget.h>
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
#include <models/logstash/logstashmodel.h>

using namespace logger;

#include "forms/connectionswidget.h"
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

void LogFileModelTest::testGetData()
{
	LogFileModel model(nullptr);
	FileConditions conditions;
	conditions.fileName("c:/logs/logviewer.log");
	model.setQueryConditions(conditions);
	model.query(conditions);

	for (int i = 1;; i++) {
		auto index = model.createIndex(std::rand() % model.rowCount(), 0);
		model.data(index, Qt::DisplayRole);
		if (i % 1000 == 0)
			qDebug() << i;
		if (i % 100000 == 0)
			model.query(conditions);
	}
}

void LogStashModelTest::test()
{
	LogStashModel model(nullptr);
	LogStashConditions conditions;
	conditions.connection("localhost");
	model.setQueryConditions(conditions);
	model.query(conditions);
	for(int i = 1;;i++) {
		model.getQueryRangeFromIndex(std::rand() % model._rows);
		if(i%100000 == 0)
			model.query(conditions);
	}
}
void LogStashModelTest::testGetData()
{
	LogStashModel model(nullptr);
	LogStashConditions conditions;
	conditions.connection("localhost");
	model.setQueryConditions(conditions);
	model.query(conditions);
		
	for (int i = 1;; i++) {
		auto index = model.createIndex(std::rand() % model._rows, 0);
		model.data(index, Qt::DisplayRole);
		if (i % 1000 == 0)
			qDebug() << i;
		if (i % 100000 == 0)
			model.query(conditions);
	}
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

