#include "querydialog.h"
#include "ui_querydialog.h"
#include "settings.h"

//@TODO should be abstract
#include <models/logstash/logstashmodel.h>
#include <models/database/logsqlmodel.h>

#include <QtWidgets>
#include <QtSql/QtSql>

enum class ConnectionType
{
	Database,
	Logstash
};
struct ConnectionInfo {
	QString name;
	enum class Type {
		Database,
		Logstash
	};
	Type type;
};
Q_DECLARE_METATYPE(ConnectionInfo);

QueryDialog::QueryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QueryDialog)
{
    Settings settings;
    ui->setupUi(this);
	
	
	auto getLastItem = [this]()
	{
		QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->cbConnection->model());
		auto lastItemIndex = model->rowCount() - 1;
		return model->item(lastItemIndex);
	};
	ui->cbConnection->blockSignals(true);
	ui->cbConnection->addItem("Database Connections");
	getLastItem()->setFlags(getLastItem()->flags() & ~Qt::ItemIsSelectable);
	foreach(auto conName, settings.childGroups("connections/database")) {
		settings.connections().database(conName);
		ui->cbConnection->addItem("  " + conName, 
			QVariant().fromValue(ConnectionInfo{ conName, ConnectionInfo::Type::Database }));
	}
	
	ui->cbConnection->addItem("Logstash Connections");
	getLastItem()->setFlags(getLastItem()->flags() & ~Qt::ItemIsSelectable);
	foreach(auto conName, settings.childGroups("connections/logstash")) {
		ui->cbConnection->addItem("  " + conName, 
			QVariant().fromValue(ConnectionInfo{ conName, ConnectionInfo::Type::Logstash }));
	}
	ui->cbConnection->blockSignals(false);
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
	ConnectionInfo ci = ui->cbConnection->currentData().value<ConnectionInfo>();
	
	if (ci.type == ConnectionInfo::Type::Database) {		
		ui->cbTables->setEnabled(true);
		QSqlDatabase& db = utils::database::getDatabase(ci.name);
		ui->cbTables->clear();
		if (db.open())
			ui->cbTables->addItems(db.tables());
	} else
		ui->cbTables->setEnabled(false);
}

void QueryDialog::on_btnCancel_clicked()
{
    reject();
}

void QueryDialog::on_btnOpen_clicked()
{
	ConnectionInfo ci = ui->cbConnection->currentData().value<ConnectionInfo>();
	if (ci.type == ConnectionInfo::Type::Database) {
		SqlConditions conditions;
		conditions.modelClass("LogSqlModel");
		conditions.database(ci.name);
		conditions.connectionName(ci.name);
		conditions.tableName(ui->cbTables->currentText());
		conditions.limit(ui->editLimit->text().toInt());
		conditions.connection(ci.name);
		_QueryOptions = conditions;
	}
	if (ci.type == ConnectionInfo::Type::Logstash) {
		LogStashConditions conditions;
		conditions.modelClass("LogStashModel");
		//conditions.host(ci.name);
		conditions.limit(ui->editLimit->text().toInt());
		conditions.connection(ci.name);
		_QueryOptions = conditions;
	}

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

