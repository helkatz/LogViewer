#include <core/settings.h>
#include <interfaces/Factory.h>
#include <gui/settings_dialog.h>
#include <ui/ui_settings_dialog.h>
#include <gui/settings_generell.h>
#include <gui/settings_templates.h>
//#include <gui/columnizerwidget.h>

namespace {
	struct ItemData : public QTreeWidgetItem
	{
		//using QTreeWidgetItem::QTreeWidgetItem;
		ItemData(const QString& name, QTreeWidgetItem *parent):
			name(name)
		{
			parent->addChild(this);
			setText(0, QObject::trUtf8(name.toUtf8()));
		}

		ItemData(const QString& name, QTreeWidget *parent)
			: name(name)
			, QTreeWidgetItem(parent)
		{
			setText(0, QObject::trUtf8(name.toUtf8()));
		}
		QWidget *boundWidget = nullptr;
		QString name;
	};

	ItemData* createChild(ItemData* parent, QStringList nodes)
	{
		if (nodes.size() == 0)
			return parent;
		if (nodes.at(0).length() == 0) {
			return parent;
		}
		parent->setFlags(parent->flags() & ~Qt::ItemIsSelectable);
		for (int ic = 0; ic < parent->childCount(); ++ic) {
			auto child = dynamic_cast<ItemData *>(parent->child(ic));
			if (child->name == nodes.at(0))
				return createChild(child, nodes.mid(1));
		}
		auto item = new ItemData(nodes.at(0), parent);
		return createChild(item, nodes.mid(1));
	}

	ItemData* createChild(QTreeWidget *parent, QStringList nodes)
	{
		if (nodes.size() == 0)
			return nullptr;
		if (nodes.size() == 1)
			return new ItemData(nodes.at(0), parent);

		for (int i = 0; i < parent->topLevelItemCount(); ++i) {
			auto item = dynamic_cast<ItemData *>(parent->topLevelItem(i));
			if (item->name != nodes.at(0))
				continue;
			return createChild(item, nodes.mid(1));
		}
		auto item = new ItemData(nodes.at(0), parent);
		return createChild(item, nodes.mid(1));
	};
}
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

	
	auto addTreeNode = [this](const QString& name, QWidget *content) {

		auto nodeNames = name.split(".");

		ItemData *item = createChild(ui->settingsTree, nodeNames);
		item->boundWidget = content;

		content->hide();
		content->setWindowFlags(Qt::Widget);

		widgets_.push_back(content);
		return item;
	};
	
	addTreeNode("Generell", new GenerellWidget(this));
	//addTreeNode("connections", new ConnectionsWidget(ui->content));
	addTreeNode("Templates", new TemplatesWidget(this));

	connect(ui->settingsTree, &QTreeWidget::currentItemChanged, this, 
		[this](QTreeWidgetItem *current, QTreeWidgetItem *previous)
	{
		//auto widget = current->boudata(0, Qt::UserRole).value<QWidget *>();
		auto widget = dynamic_cast<ItemData *>(current);
		if (!widget) 
			return;

		if (widget->boundWidget) {
			ui->gridLayout->addWidget(widget->boundWidget, 0, 1, 1, 1);
			widget->boundWidget->show();
		}

		widget = dynamic_cast<ItemData *>(previous);
		if(widget && widget->boundWidget)
			widget->boundWidget->hide();
	});
	
	auto additionalWidgets = plugin_factory::Factory::Get("settings.*");
	for (auto object : additionalWidgets) {
		auto widget = qobject_cast<QWidget *>(object->create(this));		
		if (widget == nullptr)
			continue;
		widget->setParent(ui->content);
		auto nodes = object->category().split(".");
		nodes.push_back(object->name());
		addTreeNode(nodes.mid(1).join("."), widget);
	}

	//ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok);
	//	connect(ui->settingsTree, &QTreeWidget::currentItemChanged, this,
	//		[this](QTreeWidgetItem* current, QTreeWidgetItem* previous)
	//		{
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::reload()
{
    return true;
}

void SettingsDialog::on_okButton_clicked()
{

}

void SettingsDialog::on_cancelButton_clicked()
{
	this->close();
}

void SettingsDialog::on_saveButton_clicked()
{
	emit saveSettings();
	//appSettings().saveCache();
}

