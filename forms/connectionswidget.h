#ifndef CONNECTIONSWIDGET_H
#define CONNECTIONSWIDGET_H

#include <QDialog>
#include <QSettings>
#include <qdialogbuttonbox.h>

#include <qcombobox.h>
namespace Ui {
class ConnectionsWidget;
}

class AbstractTab;
class DatabaseTab;
class LogstashTab;
class ConnectionsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionsWidget(QDialog *parent = 0);
    ~ConnectionsWidget();

	AbstractTab *getTabWidget();
private:
private:
	DatabaseTab *databaseTab;
	LogstashTab *logstashTab;
	QTabWidget *tabWidget;
    //Ui::ConnectionsWidget *ui;
    void saveSettings();
    void loadSettings();
};

class AbstractTab: public QWidget
{
public:
	explicit AbstractTab(QWidget *parent = 0);
	virtual void testConnection() {};
	virtual void deleteConnection() = 0;
	virtual void newConnection() = 0;
protected:
	
};
class DatabaseTab : public AbstractTab
{	
	Q_OBJECT
public:
	explicit DatabaseTab(QWidget *parent = 0);
	void saveSettings();
	void loadSettings();
	void testConnection();
	void deleteConnection();
	void newConnection();
private:
	QComboBox *nameCombo;
	QComboBox *driverCombo;
	QLineEdit *hostEdit;
	QLineEdit *databaseEdit;
	QLineEdit *usernameEdit;
	QLineEdit *passwordEdit;
};

class LogstashTab : public AbstractTab
{
	Q_OBJECT
public:
	explicit LogstashTab(QWidget *parent = 0);
	void saveSettings();
	void loadSettings();
	void testConnection();
	void deleteConnection();
	void newConnection();
private:
	QComboBox *nameCombo;
	QLineEdit *hostEdit;
	QLineEdit *usernameEdit;
	QLineEdit *passwordEdit;

};
#endif // CONNECTIONSWIDGET_H
