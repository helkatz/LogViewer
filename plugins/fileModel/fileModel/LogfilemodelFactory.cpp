#include "LogFileModelFactory.h"
#include <gui/ColumnizerWidget.h>
#include <gui/GenerellSettings.h>
#include <gui/logview/LogView.h>
#include <core/common.h>

#include <qfiledialog.h>

QString LogFileOpener::name() const
{
	return "LogFileOpener";
};

QWidget *LogFileOpener::createWidget(QWidget *parent)
{
	return new QFileDialog(parent);
}

QList<QueryParams> LogFileOpener::exec()
{
	QFileDialog d;
	QList<QueryParams> ret;
	QStringList fileNames = d.getOpenFileNames();
	foreach(QString fileName, fileNames) {
		FileQueryParams qc;
		qc.fileName(fileName);
		qc.modelClass("LogFileModel");
		ret.push_back(qc);
	}
	return ret;
}

LogFilePlugin::LogFilePlugin()
{
	registerType<ColumnizerWidget>("settings.Plugins.LogFile", "Columnizer");
	registerType<GenerellWidget>("settings.Plugins.LogFile", "");
	registerType<LogFileModel>("models", "LogFileModel");
	registerType<LogFileOpener>("openers", "File");
}

