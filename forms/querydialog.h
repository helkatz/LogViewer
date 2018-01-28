#ifndef QUERYDIALOG_H
#define QUERYDIALOG_H

#include <QDialog>
//#include "../QueryOptions.h"
#include "logsqlmodel.h"
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
    Conditions getQueryOptions() const
        { return _QueryOptions; }
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
    Conditions _QueryOptions;
};

#endif // QUERYDIALOG_H
