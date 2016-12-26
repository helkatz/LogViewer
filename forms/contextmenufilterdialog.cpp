#include "contextmenufilterdialog.h"
#include "ui_contextmenufilterdialog.h"
#include "settings.h"
#include <QSqlDatabase>
ContextMenuFilterDialog::ContextMenuFilterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ContextMenuFilterDialog)
{
    ui->setupUi(this);
    loadFilter();
}

ContextMenuFilterDialog::~ContextMenuFilterDialog()
{
    delete ui;
}

void ContextMenuFilterDialog::on_cbFilter_currentIndexChanged(const QString &arg1)
{
    QString name = ui->cbFilter->currentText();
    ui->editFilter->setPlainText(settings.filters(name).sql());
    ui->editLimit->setText(QString("%1").arg(settings.filters(name).limit()));
}

void ContextMenuFilterDialog::loadFilter()
{
    ui->cbFilter->clear();
    ui->cbFilter->addItems(settings.childGroups("filters"));
}

void ContextMenuFilterDialog::on_btnFilterSave_clicked()
{
    QString name = ui->cbFilter->currentText();
    if(name.length()) {
        settings.filters(name).limit(ui->editLimit->text().toInt());
        settings.filters(name).sql(ui->editFilter->toPlainText());
        loadFilter();
    }
}

void ContextMenuFilterDialog::on_btnFilterRemove_clicked()
{
    QString name = ui->cbFilter->currentText();
    if(name.length()) {
        settings.filters(name).remove();
    }
}

QString ContextMenuFilterDialog::filter() const
{
    return ui->editFilter->toPlainText();
}

void ContextMenuFilterDialog::filter(const QString& filter)
{
   ui->editFilter->setPlainText(filter);
}

int ContextMenuFilterDialog::limit() const
{
    return ui->editLimit->text().toInt();
}

void ContextMenuFilterDialog::limit(int limit)
{
    ui->editLimit->setText(QString("%1").arg(limit));
}

void ContextMenuFilterDialog::select(QString name)
{
    ui->cbFilter->setCurrentIndex(ui->cbFilter->findText(name));
}

void ContextMenuFilterDialog::on_btnOpenFiltered_clicked()
{
    /*_QueryOptions.queryString = ui->editFilter->toPlainText();
    _QueryOptions.limit = ui->editLimit->text().toInt();*/
    emit query(filter(), limit());
    //accept();
}
