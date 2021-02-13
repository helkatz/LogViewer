#include <interfaces/logmodel.h>
#include <interfaces/Factory.h>
#include <qapplication.h>
#include <qdir.h>
#include <qpluginloader.h>


namespace plugin_factory {
	QList<QSharedPointer<CreatorBase>> Factory::registered_;
	void Factory::LoadPlugins()
	{
		QDir pluginsDir(QApplication::instance()->applicationDirPath());
		pluginsDir.cd("plugins");
		foreach(auto fileInfo, pluginsDir.entryInfoList(QDir::Files)) {
			QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileInfo.fileName()));
			auto factory = qobject_cast<Factory *>(pluginLoader.instance());
		}
	}
}
