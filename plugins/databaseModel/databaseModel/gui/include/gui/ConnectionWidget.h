#ifndef ConnectionWidget_H
#define ConnectionWidget_H

#include <QDialog>
#include <QMessageBox>
#include <boost/optional.hpp>
namespace Ui {
	class ConnectionWidgetUi;
}
class ConnectionWidget: public QDialog
{
    Q_OBJECT;

    void disableButtons(boost::optional<bool> add
        , boost::optional<bool> del, boost::optional<bool> save);
public:
    ConnectionWidget(QWidget *parent = 0);
    ~ConnectionWidget();

    QString driverName() const;
    QString databaseName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    int port() const;
    bool useInMemoryDatabase() const;

private slots:

    void saveSettings();
    void loadSettings();
    void on_nameCombo_currentIndexChanged(const QString& text);
    void on_nameCombo_editTextChanged(const QString& text);
    void on_addButton_clicked();
    void on_saveButton_clicked();
    void on_deleteButton_clicked();
    void on_testButton_clicked();
	void on_dbCheckBox_clicked();

private:
    Ui::ConnectionWidgetUi *ui;
};

#endif
