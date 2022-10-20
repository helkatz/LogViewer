#include "../Settings.h"
#include "../Helper.h"
#include "../DatabaseModel.h"
#include <gui/OpenerDialog.h>
#include <ui/ui_openerdialog.h>
#include <utils/utils.h>

#include <QtWidgets>
#include <QtSql/QtSql>


DatabaseOpenerDialog::DatabaseOpenerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenerDialog)
{
	auto settings = appSettings()->as<DatabaseSettings>();
    ui->setupUi(this);
	foreach(auto conName, settings.connectionsList()) {
		ui->connections->addItem(conName.first);
	}
    //loadFilter();
}

DatabaseOpenerDialog::~DatabaseOpenerDialog()
{
    delete ui;
}

QString DatabaseOpenerDialog::connection() const
{
    return ui->connections->currentText();
}

QString DatabaseOpenerDialog::table() const
{
    return ui->tables->currentText();
}

int DatabaseOpenerDialog::limit() const
{
    return 10000;
}

void DatabaseOpenerDialog::loadFilter()
{
    //Settings s;
    //ui->cbFilter->clear();
    //ui->cbFilter->addItems(s.childGroups("filters"));
}

void DatabaseOpenerDialog::on_connections_currentIndexChanged(const QString &name)
{
	ui->tables->setEnabled(true);
	QSqlDatabase& db = helper::getDatabase(name);
	ui->tables->clear();
	if (db.open())
		ui->tables->addItems(db.tables());
}

void DatabaseOpenerDialog::on_cancel_clicked()
{
    reject();
}

void DatabaseOpenerDialog::on_open_clicked()
{
	qp.modelClass("DatabaseModel");
	qp.database(ui->connections->currentText());
	qp.connectionName(ui->connections->currentText());
	qp.tableName(ui->tables->currentText());
	qp.limit(ui->editLimit->text().toInt());
	qp.connection(ui->connections->currentText());
    
    accept();
}

void DatabaseOpenerDialog::on_btnOpenFiltered_clicked()
{
    qp.queryString(ui->editFilter->toPlainText());
    qp.limit(ui->editLimit->text().toInt());
}

void DatabaseOpenerDialog::on_btnFilterSave_clicked()
{
    //Settings s;
    //QString name = ui->cbFilter->currentText();
    //if(name.length()) {
    //    s.filters(name).limit(ui->editLimit->text().toInt());
    //    s.filters(name).sql(ui->editFilter->toPlainText());
    //    loadFilter();
    //}
}

void DatabaseOpenerDialog::on_btnFilterRemove_clicked()
{
    //Settings s;
    //QString name = ui->cbFilter->currentText();
    //if(name.length()) {
    //    s.filters(name).remove();
    //}
}

void DatabaseOpenerDialog::on_cbFilter_currentIndexChanged(const QString &)
{
    //Settings s;
    //QString name = ui->cbFilter->currentText();
    //ui->editFilter->setPlainText(s.filters(name).sql());
    //ui->editLimit->setText(QString(s.filters(name).limit()));
}

