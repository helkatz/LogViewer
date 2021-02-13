#include "WidgetsCollection.h"

// plugin
#include "NameChooserPlugin.h"
#include "QueryFilterPlugin.h"
// Qt
#include <QtCore/QtPlugin>

WidgetsCollection::WidgetsCollection(QObject* parent)
	: QObject(parent)
{
	mWidgetFactories.append(new NameChooserPlugin(this));
	mWidgetFactories.append(new QueryFilterPlugin(this));
}

QList<QDesignerCustomWidgetInterface*> WidgetsCollection::customWidgets() const
{
	return mWidgetFactories;
}

//Q_EXPORT_PLUGIN2(common_widgets, WidgetsCollection)