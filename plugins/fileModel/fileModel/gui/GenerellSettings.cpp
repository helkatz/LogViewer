#include <gui/GenerellSettings.h>
//#include <ui/ui_generell.h>
#include <ui/ui_generellsettings.h>
#include <core/settings.h>
#include <common/Logger.h>
GenerellWidget::GenerellWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenerellWidget)
{
    ui->setupUi(this);
	connect(parent, SIGNAL(saveSettings()), this, SLOT(saveSettings()));

	loadSettings();
}

GenerellWidget::~GenerellWidget()
{
    delete ui;
}

void GenerellWidget::saveSettings()
{
	settings.virtualRowsMinSize(ui->virtualRowsMinSizeEdit->text().toInt());
}

void GenerellWidget::loadSettings()
{
	ui->virtualRowsMinSizeEdit->setText(QString("%1").arg(settings.virtualRowsMinSize()));
}


