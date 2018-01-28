#ifndef ConnectionsLogstashWidget_H
#define ConnectionsLogstashWidget_H

#include <QDialog>
#include <QSettings>
namespace Ui {
class ConnectionsLogstashWidget;
}

class ConnectionsLogstashWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionsLogstashWidget(QDialog *parent = 0);
    ~ConnectionsLogstashWidget();

private slots:

    void on_btnSave_clicked();

    void on_btnCancel_clicked();

    void on_db_name_currentTextChanged(const QString &arg1);

    void on_db_name_currentIndexChanged(int index);

	void on_ls_name_currentIndexChanged(int index);

    void on_btnDelete_clicked();

    void on_btnTest_clicked();

private:
    Ui::ConnectionsLogstashWidget *ui;
    void saveSettings();
    void loadSettings();
};

#endif // ConnectionsLogstashWidget_H
