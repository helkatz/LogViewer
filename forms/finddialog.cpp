#include "finddialog.h"
#include "ui_finddialog.h"

FindDialog::FindDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FindDialog)
{
    ui->setupUi(this);
}

FindDialog::~FindDialog()
{
    delete ui;
}

void FindDialog::on_btnNext_clicked()
{
    emit find(0, ui->editSql->toPlainText(), true);
}

void FindDialog::on_btnPrev_clicked()
{
    emit find(0, ui->editSql->toPlainText(), false);
}

void FindDialog::on_bntClose_clicked()
{
    reject();
}
