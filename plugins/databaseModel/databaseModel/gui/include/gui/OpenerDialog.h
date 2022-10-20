#pragma once
#include <interfaces/LogModel.h>
#include <QDialog>
namespace Ui {
class OpenerDialog;
}

class DatabaseOpenerDialog : public QDialog
{
    Q_OBJECT

private:
    void loadFilter();
public:
    DatabaseQueryParams qp;

    explicit DatabaseOpenerDialog(QWidget *parent = 0);
    ~DatabaseOpenerDialog();
    QString connection() const;
    QString table() const;
    int limit() const;

private slots:
    void on_connections_currentIndexChanged(const QString &arg1);

    void on_open_clicked();

    void on_cancel_clicked();

    void on_btnFilterSave_clicked();

    void on_btnFilterRemove_clicked();

    void on_cbFilter_currentIndexChanged(const QString &arg1);

    void on_btnOpenFiltered_clicked();

private:
    Ui::OpenerDialog *ui;
};
