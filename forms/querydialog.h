#ifndef QUERYDIALOG_H
#define QUERYDIALOG_H

#include <QDialog>
//#include "../queryconditions.h"
#include "../logsqlmodel.h"
namespace Ui {
class QueryDialog;
}

class QueryDialog : public QDialog
{
    Q_OBJECT

private:
    void loadFilter();
public:
    explicit QueryDialog(QWidget *parent = 0);
    ~QueryDialog();
    QString connection() const;
    QString table() const;
    int limit() const;
    SqlConditions getQueryConditions() const
        { return _queryConditions; }
private slots:
    void on_cbConnection_currentIndexChanged(const QString &arg1);

    void on_btnOpen_clicked();

    void on_btnCancel_clicked();

    void on_btnFilterSave_clicked();

    void on_btnFilterRemove_clicked();

    void on_cbFilter_currentIndexChanged(const QString &arg1);

    void on_btnOpenFiltered_clicked();

private:
    Ui::QueryDialog *ui;
    SqlConditions _queryConditions;
};

#endif // QUERYDIALOG_H
