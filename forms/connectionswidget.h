#ifndef CONNECTIONSWIDGET_H
#define CONNECTIONSWIDGET_H

#include <QDialog>
#include <QSettings>
namespace Ui {
class ConnectionsWidget;
}

class ConnectionsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionsWidget(QDialog *parent = 0);
    ~ConnectionsWidget();

private slots:

    void on_btnSave_clicked();

    void on_btnCancel_clicked();

    void on_cbName_currentTextChanged(const QString &arg1);

    void on_cbName_currentIndexChanged(int index);

    void on_btnDelete_clicked();

    void on_btnTest_clicked();

private:
    Ui::ConnectionsWidget *ui;
    void saveSettings();
    void loadSettings();
};

#endif // CONNECTIONSWIDGET_H
