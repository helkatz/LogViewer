#include <gui/mainwindow.h>
#include <ui/ui_mainwindow.h>

#include <gui/logview/LogView.h>

#include <gui/aboutdialog.h>
#include <gui/settings_dialog.h>
//#include <gui/columnizerwidget.h>
#include <gui/statusbar.h>
#include <gui/finddialog.h>

#include <core/settings.h>

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
//MainWindow *MainWindow::_instance = NULL;
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
    mdiArea->setViewMode(QMdiArea::TabbedView);
    QLocale curLocale(QLocale("de_DE"));
    QLocale::setDefault(curLocale);
    this->setLocale(curLocale);
	QTabBar *m_pMdiAreaTabBar = NULL;
	
	setTabbedView(false);
	//createDockWindows();
	for (auto& creator : plugin_factory::Factory::Get("openers")) {
		auto action = new QAction(tr(creator->name().toStdString().c_str()), this);
		ui->menuOpen->addAction(action);

		connect(action, &QAction::triggered, this, [creator, this]() {
			auto opener = dynamic_cast<plugin_factory::LogOpener *>(creator->create(nullptr));
			auto conditions = opener->exec();
			for(auto& c: conditions) {
				LogWindow::CreateParams params{};
				params.queryParams = c;
				params.determineTemplateWindow = true;
				createLogWindow(params);
			}
		});
	}
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
	// necessary for teardown
	static MainWindow& instance = *new MainWindow{};
	return instance;
}

QMenu* MainWindow::openMenu()
{
	return ui->menuOpen;
};

void MainWindow::updateMenu()
{
	QMdiSubWindow* wnd = mdiArea->activeSubWindow();
	LogWindow* logView = qobject_cast<LogWindow*>(wnd ? wnd->widget() : NULL);
	if (logView) {
		ui->actionFollowMode->blockSignals(true);
		ui->actionFollowMode->setChecked(logView->getLogView()->followMode());
		ui->actionFollowMode->blockSignals(false);
	}
}

void MainWindow::addMenuItem(QAction *action)
{
	menuBar()->addAction(action);
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
    QRect rec = QApplication::desktop()->screenGeometry();

    // moves only to pos when the screen exists for the stored position
	// QDesktopWidget::screenNumber(window()->pos())
	auto dt = QApplication::desktop();
	
	auto w = dt->width();
	auto windowRight = appSettings().mainWindow().pos().x() + appSettings().mainWindow().size().width();
	auto h = dt->height();
	auto windowBottom = appSettings().mainWindow().pos().y() + appSettings().mainWindow().size().height();
	if (dt->width() >= windowRight && dt->height() >= windowBottom)
		move(appSettings().mainWindow().pos());
	else
		move(20, 20);
	int screenNumber = QApplication::desktop()->screenNumber(appSettings().mainWindow().pos());

    if(false && screenNumber >= 0)
		move(appSettings().mainWindow().pos());

	resize(appSettings().mainWindow().size());
	

    for(auto it: appSettings().windowsList()) {
		auto& sWindow = it.second;
		try {
			LogWindow::CreateParams params;
			params.settings = sWindow;
			LogWindow *logWindow = createLogWindow(params);
			if (logWindow == nullptr)
				continue;
			QMdiSubWindow *frame = qobject_cast<QMdiSubWindow *>(logWindow->parentWidget());
			if (frame) {
				frame->resize(sWindow.size());
				frame->move(sWindow.pos());
				if (sWindow.maximized() == true)
					frame->showMaximized();
				if (sWindow.minimized() == true)
					frame->showMinimized();
			}
		}
		catch (std::exception&) {

		}
    }
	// set this here after sub windows creation
	setTabbedView(appSettings().mainWindow().tabbedView());
    log_trace(5)<<"done";
}

void MainWindow::writeSettings()
{
	auto& s = appSettings();
    int i = 1;
	s.mainWindow().pos(pos());
	s.mainWindow().size(size());
	s.mainWindow().tabbedView(mdiArea->viewMode() == QMdiArea::TabbedView);

	// remove all views from config and rewrite it
    s.windows().remove();

    foreach (QMdiSubWindow *frame, mdiArea->subWindowList()) {
		auto s = appSettings().windows(i);
        LogWindow *logView = qobject_cast<LogWindow *>(frame->widget());
        logView->writeSettings(s);
		// save frame 
		s.size(frame->size());
        s.pos(frame->pos());
        s.maximized(frame->isMaximized());
        s.minimized(frame->isMinimized());
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

LogWindow *MainWindow::createLogWindow(LogWindow::CreateParams& params)
{
    LogWindow *window = LogWindow::create(params);
    addLogWindow(window, true);
    return window;
}

void MainWindow::subWindowChanged(QMdiSubWindow* window)
{
	qDebug() << "subWindowChanged";
    ui->actionFind->setEnabled(window != NULL);
    refreshWindowTitle();
	updateMenu();
    if(window != NULL) {
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

void MainWindow::on_actionTabbed_toggled(bool enable)
{
	setTabbedView(enable);
}
