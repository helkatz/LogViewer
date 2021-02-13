#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

namespace Ui {
class FindDialog;
}

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = 0);
    ~FindDialog();

private slots:
    void on_btnNext_clicked();

    void on_btnPrev_clicked();

    void on_bntClose_clicked();
signals:
    int find(int fromPos, QString where, bool down);
private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
