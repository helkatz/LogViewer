#include "DatabaseModelFactory.h"
#include <gui/MainWindow.h>
#include <gui/logview/LogView.h>
#include <gui/ConnectionWidget.h>
#include <gui/OpenerDialog.h>
#include <core/common.h>

#include <QLineEdit>
class DatabaseOpenerDialog : public OpenerDialog
{
	QLineEdit* database;
	QLineEdit* tableName;
	QLineEdit* connectionName;

public:
	using OpenerDialog::OpenerDialog;
	DatabaseOpenerDialog(QWidget* parent)
		: OpenerDialog(parent)
	{
		properties()->addRow("connectionName", connectionName = new QLineEdit(this));
		properties()->addRow("database", database = new QLineEdit(this));
		properties()->addRow("tableName", tableName = new QLineEdit(this));
	}

	DatabaseOpenerDialog conditions()
	{
		DatabaseQueryParams c;
		c.database(database->text());
		c.connectionName(connectionName->text());
		c.database(database->text());
		c.tableName(tableName->text());
	}
};

QString DatabaseOpener::name() const
{
	return "DatabaseOpenerDialog";
};

QWidget* DatabaseOpener::createWidget(QWidget* parent)
{
	return new DatabaseOpenerDialog(parent);
}

QList<QueryParams> DatabaseOpener::exec()
{
	DatabaseOpenerDialog d(nullptr);
	auto dlgRet = d.exec();
	QList<QueryParams> ret;
	return ret;
}

DatabasePlugin::DatabasePlugin()
{
	registerType<ConnectionWidget>("settings.Plugins.DatabaseLog", "Connections");
	//registerType<ColumnizerWidget>("settings.connections", "LogFileModel");
	registerType<LogDatabaseModel>("models", "DatabaseModel");
	registerType<DatabaseOpener>("openers", "Database");
}
