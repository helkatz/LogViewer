#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "queryconditions.h"
#include "logmodel.h"
#include <QMainWindow>
#include <QMdiArea>

#include "ui_mainwindow.h"
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

class MainWindow : public QMainWindow
{
	friend class LogWindowTest;
    Q_OBJECT
    explicit MainWindow(QWidget *parent = 0);
    static MainWindow *_instance;
	QTabBar *_tabBar;
public:
    static MainWindow& instance();
    const QMdiArea& getArea() const { return *mdiArea; }
    void refreshWindowTitle();
    ~MainWindow();

private slots:

    void subWindowChanged(QMdiSubWindow* window);

    void on_actionOpen_triggered();

    void on_actionAbout_triggered();

    void on_actionExit_triggered();

    void on_actionSettings_triggered();

	void on_actionColumnizer_triggered();

    void on_actionFollowMode_changed();

    void on_actionFollowMode_toggled(bool);

    void on_actionFind_triggered();

    void on_actionSave_Desktop_triggered();

    void on_actionDatabase_Log_triggered();

    void on_actionFile_Log_triggered();

	void on_actionLogstash_triggered();

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
    LogWindow *createLogView(const Conditions& qc, bool useTemplate = false);
	LogWindow *createLogView(const Conditions &qc, LogView *templateWIndow);
    LogWindow *createLogView(const QString& settingsPath);
    LogWindow *createFromSettigs(const QString& settingsPath);
	bool setTabbedView(bool enable);
protected slots:
    //LogWindow *createLogView(LogSqlModel *model, const QString &settingsPath = "");
};

#endif // MAINWINDOW_H
