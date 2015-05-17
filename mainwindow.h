#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "queryconditions.h"
#include "logmodel.h"
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
class MainWindow : public QMainWindow
{
    Q_OBJECT
    explicit MainWindow(QWidget *parent = 0);
    static MainWindow *_instance;
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

    void on_actionFollowMode_changed();

    void on_actionFollowMode_toggled(bool arg1);

    void on_actionFind_triggered();

    void on_actionSave_Desktop_triggered();

    void on_actionDatabase_Log_triggered();

    void on_actionFile_Log_triggered();

private:
    Ui::MainWindow *ui;
    QMdiArea *mdiArea;
    LogUpdater *logUpdater;

    void writeSettings();
    LogView *activeLogView();
    QMdiSubWindow *addLogWindow(LogWindow *window, bool autoPosAndSize);
public slots:
    void readSettings();
    LogWindow *createLogView(const Conditions& qc, bool useTemplate = false);
    LogWindow *createLogView(const QString& settingsPath);
    LogWindow *createFromSettigs(const QString& settingsPath);
protected slots:
    //LogWindow *createLogView(LogSqlModel *model, const QString &settingsPath = "");
};

#endif // MAINWINDOW_H
