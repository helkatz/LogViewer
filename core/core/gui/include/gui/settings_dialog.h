#pragma once
#include <core/common.h>
#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class SettingsDialog;
}


class CORE_API SettingsDialog : public QDialog
{
    Q_OBJECT;
    
    QList<QWidget*> widgets_;

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    bool reload();

signals:
    void saveSettings();

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
    void on_saveButton_clicked();

private:
    Ui::SettingsDialog *ui;
};

