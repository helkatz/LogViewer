#include "mainwindow.h"
#include <QApplication>
#include "forms/QueryDialog.h"
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
#include "logview.h"
int main(int argc, char *argv[])
{
    Settings::setOrganisation("ACOM");
    Settings::setApplication("LogViewer");
    _putenv("QT_MESSAGE_PATTERN=\"[%{type}] %{appname} %{threadid} - %{message}\"");
    _putenv("QT_FATAL_WARNINGS=");
    Q_INIT_RESOURCE(LogViewer);

    QApplication a(argc, argv);

    MainWindow::instance().show();
    MainWindow::instance().readSettings();
    //QErrorMessage::qtHandler();
    return a.exec();
}
