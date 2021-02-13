#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <core/common.h>
#include <interfaces/LogModel.h>
#include <gui/logview/LogView.h>
#include <interfaces/Factory.h>

#include <QMainWindow>
#include <QMdiArea>

class QMdiArea;
class LogView;
class LogSqlModel;
class LogUpdater;
namespace Ui {
class MainWindow;
}
class LogWindow;
class MdiArea : public QMdiArea
{
	QTabBar *_tabBar;
public:
	MdiArea(QWidget *parent = 0);
	bool setTabbedView(bool enable);
};

class CORE_API MainWindow : public QMainWindow
{
	friend class LogWindowTest;
    Q_OBJECT
    explicit MainWindow(QWidget *parent = 0);    
	QTabBar *_tabBar;
public:
    static MainWindow& instance();

    const QMdiArea& getArea() const { return *mdiArea; }
   
	void createDockWindows();

    ~MainWindow();

	QMenu* openMenu();

    void updateMenu();

private slots:

	void addMenuItem(QAction *action);

	void refreshWindowTitle();

    void subWindowChanged(QMdiSubWindow* window);

    void on_actionAbout_triggered();

    void on_actionExit_triggered();

    void on_actionSettings_triggered();

    void on_actionFollowMode_toggled(bool);

    void on_actionFind_triggered();

    void on_actionSave_Desktop_triggered();

	void on_actionTabbed_toggled(bool);
private:
    Ui::MainWindow *ui;
    MdiArea *mdiArea;
    LogUpdater *logUpdater;

    void writeSettings();
    LogView *activeLogView();
    QMdiSubWindow *addLogWindow(LogWindow *window, bool autoPosAndSize);
public slots:
    void readSettings();
    LogWindow *createLogWindow(LogWindow::CreateParams& params);
	bool setTabbedView(bool enable);
protected slots:
};

#endif // MAINWINDOW_H
