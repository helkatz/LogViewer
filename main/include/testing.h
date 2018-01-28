#include "mainwindow.h"
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


#include "forms/connectionswidget.h"
struct WidgetTest
{
	static void ColumnizerWidgetTest();
	static void ConnectionsWidgetTest();
};
class LogFileModelTest
{
public:
	void testGetData();
};

class LogStashModelTest
{
public:

	void test();
	void testGetData();
};

class LogWindowTest : public QThread
{
	Q_OBJECT
signals:
	void scrolltable(QModelIndex index);
private slots:
	void run();
public:
	LogWindowTest();
};

