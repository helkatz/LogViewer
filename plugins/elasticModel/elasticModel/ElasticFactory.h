#include "elasticmodel.h"
#include <interfaces/Factory.h>

class ElasticOpener : public plugin_factory::LogOpener
{
	Q_OBJECT
public:
	using LogOpener::LogOpener;
	QString name() const override;

	QWidget *createWidget(QWidget *parent) override;

	QList<QueryParams> exec() override;
};

class ElasticPlugin : public QObject, public plugin_factory::Factory
{
	Q_OBJECT;
	Q_PLUGIN_METADATA(IID "hk.LogViewer.PluginFactory" FILE "ElasticPlugin.json");
	Q_INTERFACES(plugin_factory::Factory);
public:
	ElasticPlugin();
};