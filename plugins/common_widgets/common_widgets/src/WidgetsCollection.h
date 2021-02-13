#pragma once
#include <QtDesigner/QDesignerCustomWidgetCollectionInterface>
#include <QtCore/QObject>

class WidgetsCollection : public QObject, public QDesignerCustomWidgetCollectionInterface
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface")
	Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

	public:
	   explicit WidgetsCollection(QObject* parent = 0);

	public: // QDesignerCustomWidgetCollectionInterface API
		virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;
	private:
		QList<QDesignerCustomWidgetInterface*> mWidgetFactories;
};
