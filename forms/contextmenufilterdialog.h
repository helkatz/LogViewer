#ifndef CONTEXTMENUFILTERDIALOG_H
#define CONTEXTMENUFILTERDIALOG_H

#include "../settings.h"
#include <QDialog>

namespace Ui {
class ContextMenuFilterDialog;
}

class ContextMenuFilterDialog : public QDialog
{
    Q_OBJECT
    Settings settings;
public:
    explicit ContextMenuFilterDialog(QWidget *parent = 0);
    ~ContextMenuFilterDialog();
    QString filter() const;
    void filter(const QString& filter);
    int limit() const;
    void limit(int limit);
    void select(QString name);
signals:
    void query(QString, int);
private slots:
    void on_cbFilter_currentIndexChanged(const QString &arg1);

    void on_btnFilterSave_clicked();

    void on_btnFilterRemove_clicked();

    void on_btnOpenFiltered_clicked();


private:
    Ui::ContextMenuFilterDialog *ui;
    void loadFilter();
};

#endif // CONTEXTMENUFILTERDIALOG_H
