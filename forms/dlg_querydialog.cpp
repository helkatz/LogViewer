#include "querydialog.h"
#include "ui_querydialog.h"
#include "settings.h"
#include "Utils/utils.h"
#include <QtWidgets>
#include <QtSql/QtSql>
QueryDialog::QueryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryDialog)
{
    Settings settings;
    ui->setupUi(this);
    ui->cbConnection->addItems(settings.childGroups("connections"));
    loadFilter();
}

QueryDialog::~QueryDialog()
{
    delete ui;
}

QString QueryDialog::connection() const
{
    return ui->cbConnection->currentText();
}

QString QueryDialog::table() const
{
    return ui->cbTables->currentText();
}

int QueryDialog::limit() const
{
    return 10000;
}

void QueryDialog::loadFilter()
{
    Settings s;
    ui->cbFilter->clear();
    ui->cbFilter->addItems(s.childGroups("filters"));
}

void QueryDialog::on_cbConnection_currentIndexChanged(const QString &arg1)
{
	QSqlDatabase& db = utils::database::getDatabase(arg1);
    ui->cbTables->clear();
    if(db.open())
        ui->cbTables->addItems(db.tables());
}

void QueryDialog::on_btnCancel_clicked()
{
    reject();
}

void QueryDialog::on_btnOpen_clicked()
{
    _QueryOptions.modelClass("LogSqlModel");
    _QueryOptions.database(ui->cbConnection->currentText());
    _QueryOptions.connectionName(ui->cbConnection->currentText());
    _QueryOptions.tableName(ui->cbTables->currentText());
    _QueryOptions.limit(ui->editLimit->text().toInt());
    accept();
}

void QueryDialog::on_btnOpenFiltered_clicked()
{
    _QueryOptions.queryString(ui->editFilter->toPlainText());
    _QueryOptions.limit(ui->editLimit->text().toInt());
}

void QueryDialog::on_btnFilterSave_clicked()
{
    Settings s;
    QString name = ui->cbFilter->currentText();
    if(name.length()) {
        s.filters(name).limit(ui->editLimit->text().toInt());
        s.filters(name).sql(ui->editFilter->toPlainText());
        loadFilter();
    }
}

void QueryDialog::on_btnFilterRemove_clicked()
{
    Settings s;
    QString name = ui->cbFilter->currentText();
    if(name.length()) {
        s.filters(name).remove();
    }
}

void QueryDialog::on_cbFilter_currentIndexChanged(const QString &)
{
    Settings s;
    QString name = ui->cbFilter->currentText();
    ui->editFilter->setPlainText(s.filters(name).sql());
    ui->editLimit->setText(QString(s.filters(name).limit()));
}

