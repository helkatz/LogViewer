#include "DatabaseModel.h"
#include <interfaces/Factory.h>

class DatabaseOpener : public plugin_factory::LogOpener
{
	Q_OBJECT
public:
	using LogOpener::LogOpener;
	QString name() const override;

	QWidget* createWidget(QWidget* parent) override;

	QList<QueryParams> exec() override;
};

class DatabasePlugin : public QObject, public plugin_factory::Factory
{
	Q_OBJECT;
	Q_PLUGIN_METADATA(IID "hk.LogViewer.LogModelFactory" FILE "DatabasePlugin.json");
	Q_INTERFACES(plugin_factory::Factory);
public:
	DatabasePlugin();
};