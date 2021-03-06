#include "mainwindow.h"

#include "settings.h"
#include "logview.h"

#include "logupdater.h"
#include "forms/aboutdialog.h"
#include "forms/querydialog.h"
#include "forms/settingsdialog.h"
#include "forms/columnizerwidget.h"
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
#include <QDesktopWidget>
#include <QApplication>
MainWindow *MainWindow::_instance = NULL;
static QTabBar *tabBar;
MdiArea::MdiArea(QWidget *parent):
	QMdiArea(parent)
{
	setTabsClosable(true);
	setTabsMovable(true);
	setViewMode(ViewMode::SubWindowView);
	setActivationOrder(QMdiArea::WindowOrder::ActivationHistoryOrder);
}

#include <qdockwidget.h>
#include <qlistwidget.h>
void MainWindow::createDockWindows()
{
	QDockWidget *dock = new QDockWidget(tr("Customers"), this);
	//dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::RightDockWidgetArea);
	auto customerList = new QListWidget(dock);
	customerList->addItems(QStringList()
		<< "John Doe, Harmony Enterprises, 12 Lakeside, Ambleton"
		<< "Jane Doe, Memorabilia, 23 Watersedge, Beaton"
		<< "Tammy Shea, Tiblanka, 38 Sea Views, Carlton"
		<< "Tim Sheen, Caraba Gifts, 48 Ocean Way, Deal"
		<< "Sol Harvey, Chicos Coffee, 53 New Springs, Eccleston"
		<< "Sally Hobart, Tiroli Tea, 67 Long River, Fedula");
	dock->setWidget(customerList);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	//menuBar().addAction(dock->toggleViewAction());
	dock->setTitleBarWidget(new QWidget(this));
	dock = new QDockWidget(tr("Paragraphs"), this);
	auto paragraphsList = new QListWidget(dock);
	paragraphsList->addItems(QStringList()
		<< "Thank you for your payment which we have received today."
		<< "Your order has been dispatched and should be with you "
		"within 28 days."
		<< "We have dispatched those items that were in stock. The "
		"rest of your order will be dispatched once all the "
		"remaining items have arrived at our warehouse. No "
		"additional shipping charges will be made."
		<< "You made a small overpayment (less than $5) which we "
		"will keep on account for you, or return at your request."
		<< "You made a small underpayment (less than $1), but we have "
		"sent your order anyway. We'll add this underpayment to "
		"your next bill."
		<< "Unfortunately you did not send enough money. Please remit "
		"an additional $. Your order will be dispatched as soon as "
		"the complete amount has been received."
		<< "You made an overpayment (more than $5). Do you wish to "
		"buy more items, or should we return the excess to you?");
	dock->setWidget(paragraphsList);
	addDockWidget(Qt::RightDockWidgetArea, dock);
	//viewMenu->addAction(dock->toggleViewAction());
	setTabShape(QTabWidget::TabShape::Rounded);
	setTabPosition(Qt::DockWidgetArea::AllDockWidgetAreas, QTabWidget::TabPosition::North);
	connect(customerList, SIGNAL(currentTextChanged(QString)),
		this, SLOT(insertCustomer(QString)));
	connect(paragraphsList, SIGNAL(currentTextChanged(QString)),
		this, SLOT(addParagraph(QString)));
}
bool MdiArea::setTabbedView(bool enable)
{
	QMdiArea::ViewMode mode = enable ? QMdiArea::TabbedView : QMdiArea::SubWindowView;
	bool currentMode = viewMode() == QMdiArea::TabbedView;
	if (mode == viewMode())
		return enable;
	setViewMode(mode);
	if (mode == QMdiArea::TabbedView) {
		foreach (auto wnd, children()) {
			if (QString(wnd->metaObject()->className()) == "QTabBar") {
				_tabBar = dynamic_cast<QTabBar*>(wnd);
				break;
			}
		}
		if (_tabBar) {
			int tabIdx = 0;
			foreach(auto wnd, subWindowList())
			{
				//WindowGeometry wg(wnd);
				//wnd->setUserData()
				_tabBar->setTabText(tabIdx++, QFileInfo(wnd->windowTitle()).fileName());
			}
			_tabBar->setExpanding(false);
		}
	}
	else {
		_tabBar = nullptr;
	}
	return currentMode;
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
	_tabBar(nullptr)
{
    ui->setupUi(this);
	setAttribute(Qt::WA_AlwaysShowToolTips, true);
	mdiArea = new MdiArea();
	setCentralWidget(mdiArea);
	//this->addToolBar("toolbar")->addWidget(tabBar);
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(subWindowChanged(QMdiSubWindow*)));
    //mdiArea->setViewMode(QMdiArea::TabbedView);
    QLocale curLocale(QLocale("de_DE"));
    QLocale::setDefault(curLocale);
    this->setLocale(curLocale);
	QTabBar *m_pMdiAreaTabBar = NULL;
	
	setTabbedView(false);
	createDockWindows();
}
struct WindowGeometry
{
	QPoint pos;
	QSize size;
	bool maximized;
	bool minimized;
	WindowGeometry(QWidget *w)
	{
		pos = w->pos();
		size = w->size();
		maximized = w->isFullScreen();
		minimized = w->isMinimized();
	}
	
};
bool MainWindow::setTabbedView(bool enable)
{
	ui->actionTabbed->setChecked(enable);
	return mdiArea->setTabbedView(enable);
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
    log_trace(5)<<"readSettings";
    Settings s;
    QRect rec = QApplication::desktop()->screenGeometry();

    // moves only to pos when the screen exists for the stored position
	//QDesktopWidget::screenNumber(window()->pos())
	auto dt = QApplication::desktop();
	
	auto w = dt->width();
	auto windowRight = s.mainWindow().pos().x() + s.mainWindow().size().width();
	auto h = dt->height();
	auto windowBottom = s.mainWindow().pos().y() + s.mainWindow().size().height();
	if (dt->width() >= windowRight && dt->height() >= windowBottom)
		move(s.mainWindow().pos());
	else
		move(20, 20);
	int screenNumber = QApplication::desktop()->screenNumber(s.mainWindow().pos());

    if(false && screenNumber >= 0)
		move(s.mainWindow().pos());

	resize(s.mainWindow().size());
	
	ColumnizerWidget d;
	//d.exec();
    foreach(const QString& group, s.childGroups("views")) 
	{
		try {
			LogWindow *logWindow = createLogView(s.views(group).getPath());
			if (logWindow == nullptr)
				continue;
			QMdiSubWindow *frame = qobject_cast<QMdiSubWindow *>(logWindow->parentWidget());
			if (frame) {
				frame->resize(s.views(group).size());
				frame->move(s.views(group).pos());
				if (s.views(group).maximized() == true)
					frame->showMaximized();
				if (s.views(group).minimized() == true)
					frame->showMinimized();
			}
		}
		catch (std::exception&) {

		}
    }
	// set this here after sub windows creation
	setTabbedView(s.mainWindow().tabbedView());
    log_trace(5)<<"done";
}

void MainWindow::writeSettings()
{
    Settings s;
    int i = 1;
	s.mainWindow().pos(pos());
	s.mainWindow().size(size());
	s.mainWindow().tabbedView(mdiArea->viewMode() == QMdiArea::TabbedView);

	// remove all views from config and rewrite it
    s.views().remove();

    foreach (QMdiSubWindow *frame, mdiArea->subWindowList()) {
        LogWindow *logView = qobject_cast<LogWindow *>(frame->widget());
        logView->writeSettings(s.views(i).getPath());
		// save frame 
		s.views(i).size(frame->size());
        s.views(i).pos(frame->pos());
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

LogWindow *MainWindow::createLogView(const Conditions &qc, LogView *templateWindow)
{
	Settings s;
	LogWindow *window = LogWindow::create(qc, templateWindow);
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
	qDebug() << "subWindowChanged";
    ui->actionFind->setEnabled(window != NULL);
    refreshWindowTitle();
    if(window != NULL) {
        //setWindowTitle("LogViewer : "+window->windowTitle());
		//window->chag
		if (_tabBar && _tabBar->currentIndex() >= 0) {
			_tabBar->setTabText(_tabBar->currentIndex(), QFileInfo(window->windowTitle()).fileName());
			_tabBar->setExpanding(false);
		}
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
        createLogView(d.getQueryOptions(), true);
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

void MainWindow::on_actionColumnizer_triggered()
{
	ColumnizerWidget d;
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
        createLogView(d.getQueryOptions(), true);
    }
}

//@TODO this should be encapsulated in logfile stuff like plugin or so
#include <models/file/logfilemodel.h>
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

void MainWindow::on_actionLogstash_triggered()
{
}

void MainWindow::on_actionTabbed_toggled(bool enable)
{
	setTabbedView(enable);
}
