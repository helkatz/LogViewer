#include "settingsdialog.h"
#include "settings.h"
#include "generellwidget.h"
#include "connectionswidget.h"
#include "templateswidget.h"
#include "columnizerwidget.h"

#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
	ui->tabWidget->addTab(new GenerellWidget(), tr("Generell"));
    ui->tabWidget->addTab(new ConnectionsWidget(), tr("Connections"));
    ui->tabWidget->addTab(new TemplatesWidget(), tr("Templates"));
	ui->tabWidget->addTab(new ColumnizerWidget(), tr("Columnizer"));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::reload()
{
    return true;
}


