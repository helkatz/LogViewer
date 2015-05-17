#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>
#include "ui_findwidget.h"
namespace Ui {
class FindWidget;
}

class FindWidget : public QWidget
{
    Q_OBJECT
    QMenu *columnsMenu;
public:
    explicit FindWidget(QWidget *parent = 0);
    ~FindWidget();
    void setFields(QStringList fields);
    void addSearchField(const QString& field, bool preferred);
    void setFocus();
    QStringList searchCoumns() const;
private slots:
    void on_btnDown_clicked();

    void on_btnUp_clicked();

    void on_btnClose_clicked();

private:
    Ui::FindWidget *ui;
    QString buildSql();
signals:
    int find(QString where, bool down);
};

#endif // FINDWIDGET_H
