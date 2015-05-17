#ifndef TEMPLATESWIDGET_H
#define TEMPLATESWIDGET_H

#include <QWidget>

namespace Ui {
class TemplatesWidget;
}

class TemplatesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TemplatesWidget(QWidget *parent = 0);
    ~TemplatesWidget();

private slots:
    void on_cbName_currentIndexChanged(const QString &arg1);

    void on_cbName_currentTextChanged(const QString &arg1);

    void on_btnSave_clicked();

    void on_btnDelete_clicked();

    void on_btnCancel_clicked();

private:
    Ui::TemplatesWidget *ui;
    void saveSettings();
    void loadSettings();
};

#endif // TEMPLATESWIDGET_H
