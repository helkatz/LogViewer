#include "DatabaseModelFactory.h"
#include <gui/MainWindow.h>
#include <gui/logview/LogView.h>
#include <gui/ConnectionWidget.h>
#include <gui/OpenerDialog.h>
#include <gui/OpenerDialog.h>
#include <core/common.h>

#include <QLineEdit>

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
	ret.push_back(d.qp);
	return ret;
}

DatabasePlugin::DatabasePlugin()
{
	registerType<ConnectionWidget>("settings.Plugins.DatabaseLog", "Connections");
	//registerType<ColumnizerWidget>("settings.connections", "LogFileModel");
	registerType<LogDatabaseModel>("models", "DatabaseModel");
	registerType<DatabaseOpener>("openers", "Database");
}
