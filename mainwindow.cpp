#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "forms/aboutdialog.h"
#include "forms/querydialog.h"
#include "logview.h"
#include "logsqlmodel.h"
#include "logfilemodel.h"
#include "settings.h"
#include "forms/settingsdialog.h"
#include "logupdater.h"
#include "forms/statusbar.h"
#include "forms/finddialog.h"

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSettings>
#include <QSqlError>
#include <QMessageBox>
#include <iostream>
#include <ostream>
#include <QSplitter>
#include <QFileDialog>
#include <QtWebKitWidgets/QWebView>
MainWindow *MainWindow::_instance = NULL;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mdiArea = ui->mdiArea;
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowChanged(QMdiSubWindow*)));
    //mdiArea->setViewMode(QMdiArea::TabbedView);
    QLocale curLocale(QLocale("de_DE"));
    QLocale::setDefault(curLocale);
    this->setLocale(curLocale);
}

MainWindow::~MainWindow()
{
    delete ui;
}

MainWindow& MainWindow::instance()
{
    if(!_instance) {
        _instance = new MainWindow();
    }
    return *_instance;
}

void MainWindow::refreshWindowTitle()
{
    QMdiSubWindow *window = mdiArea->activeSubWindow();
    if(window != NULL){
        setWindowTitle("LogViewer : " + window->windowTitle());
    }
    else {
        setWindowTitle("No active window");
    }
}

void MainWindow::readSettings()
{
    qDebug()<<"readSettings";
    Settings s;
    QRect rec = QApplication::desktop()->screenGeometry();

    // moves only to pos when the screen exists for the stored position
    int screenNumber = QApplication::desktop()->screenNumber(window()->pos());
    if(screenNumber >= 0)
       move(s.window().pos());

    resize(s.window().size());
    foreach(const QString& group, s.childGroups("views")) {
        //QueryConditions& qc = QueryConditions::create(s.views(group).)
        LogWindow *logWindow = createLogView(s.views(group).getPath());
        if(logWindow == NULL)
            continue;
        QMdiSubWindow *frame = qobject_cast<QMdiSubWindow *>(logWindow->parentWidget());
        if(frame) {
            frame->resize(s.views(group).size());
            frame->move(s.views(group).pos());
            //frame->setWindowTitle(s.views(group).title());
            if(s.views(group).maximized() == true)
                frame->showMaximized();
            if(s.views(group).minimized() == true)
                frame->showMinimized();
        }
    }
    qDebug()<<"done";
}

void MainWindow::writeSettings()
{
    Settings s;
    int i = 1;
    s.window().pos(pos());
    s.window().size(size());
    s.views().remove();
    foreach (QMdiSubWindow *frame, mdiArea->subWindowList()) {
        LogWindow *logView = qobject_cast<LogWindow *>(frame->widget());
        logView->writeSettings(s.views(i).getPath());
        s.views(i).size(frame->size());
        s.views(i).pos(frame->pos());
        //s.views(i).title(frame->windowTitle());
        s.views(i).maximized(frame->isMaximized());
        s.views(i).minimized(frame->isMinimized());
        i++;
    }
}

QMdiSubWindow *MainWindow::addLogWindow(LogWindow *window, bool autoPosAndSize)
{
    if(window == NULL)
        return NULL;

    QMdiSubWindow *newSubWindow = mdiArea->addSubWindow(window);
    if(newSubWindow) {
        QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow();
        if(activeSubWindow) {
            newSubWindow->resize(activeSubWindow->size());
            newSubWindow->move(activeSubWindow->pos() + QPoint(30, 30));
        }
    }
    window->show();
    mdiArea->setActiveSubWindow(newSubWindow);

    return newSubWindow;
}

LogWindow *MainWindow::createLogView(const Conditions &qc, bool useTemplate)
{
    Settings s;
    LogWindow *window = LogWindow::create(qc, useTemplate);
    addLogWindow(window, true);
    return window;
}

LogWindow *MainWindow::createLogView(const QString& settingsPath)
{
    Settings s;
    LogWindow *window = LogWindow::create(settingsPath);
    addLogWindow(window, false);
    return window;
}

LogWindow *MainWindow::createFromSettigs(const QString &settingsPath)
{
    LogWindow *window = createLogView(settingsPath);
    window->readSettings(settingsPath);
    return window;
}

void MainWindow::subWindowChanged(QMdiSubWindow* window)
{
    ui->actionFind->setEnabled(window != NULL);
    refreshWindowTitle();
    if(window != NULL){
        //setWindowTitle("LogViewer : "+window->windowTitle());
        LogView *logView = qobject_cast<LogView *>(window->widget());
        if(logView) {
            ui->actionFollowMode->setChecked(logView->followMode());
        }
    }
    else {
        //setWindowTitle("No active window");
    }
};

void MainWindow::on_actionOpen_triggered()
{
    QueryDialog d;
    Settings s;
    if(d.exec() == QueryDialog::Accepted) {
        createLogView(d.getQueryConditions(), true);
    }
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog d;
    d.exec();
}

void MainWindow::on_actionExit_triggered()
{
    writeSettings();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog d;
    d.exec();
}

void MainWindow::on_actionFollowMode_changed()
{

}

void MainWindow::on_actionFollowMode_toggled(bool arg1)
{
    QMdiSubWindow *wnd = mdiArea->activeSubWindow();
    LogWindow *logView = qobject_cast<LogWindow *>(wnd ? wnd->widget() : NULL);
    if(logView)
        logView->setFollowMode(arg1);
}

void MainWindow::on_actionFind_triggered()
{
    FindDialog d;
    QMdiSubWindow *wnd = mdiArea->activeSubWindow();
    if(false == (wnd && wnd->widget()))
        return;
    LogWindow *logView = qobject_cast<LogWindow *>(wnd ? wnd->widget() : NULL);
    logView->showFindWidget();
}

void MainWindow::on_actionSave_Desktop_triggered()
{
    writeSettings();
}

void MainWindow::on_actionDatabase_Log_triggered()
{
    QueryDialog d;
    Settings s;
    if(d.exec() == QueryDialog::Accepted) {
        createLogView(d.getQueryConditions(), true);
    }
}

void MainWindow::on_actionFile_Log_triggered()
{
    QFileDialog d;
    Settings s;
    QStringList fileNames = d.getOpenFileNames();
    foreach(QString fileName, fileNames) {
        FileConditions qc;
        qc.fileName(fileName);
        qc.modelClass("LogFileModel");
        createLogView(qc, true);
    }
}
