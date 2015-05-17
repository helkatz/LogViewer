#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "../settings.h"
#include "connectionswidget.h"
#include "templateswidget.h"
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    Settings s;
    ui->limitLineEdit->setText(QString("%1").arg(s.general().queryConditions().limit()));
    ui->followmodeCheckBox->setChecked(s.general().view().followMode());
    ui->fontComboBox->setFont(s.general().view().font());
    ui->altRowColorsCheckBox->setChecked(s.general().view().alternatingRowColors());
    //ui->coloredColumnsSpinBox->setValue(s.general().coloredColumns());
    ui->tabWidget->addTab(new ConnectionsWidget(), tr("Connections"));
    ui->tabWidget->addTab(new TemplatesWidget(), tr("Templates"));
    //QFont f = ui->fontComboBox->currentFont();

    ui->sizeSpin->setValue(s.general().view().fontSize());
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

bool SettingsDialog::reload()
{
    return true;
}

bool SettingsDialog::followMode() const
{
    return ui->followmodeCheckBox->checkState() == Qt::Checked;
}

int SettingsDialog::limit() const
{
    return ui->limitLineEdit->text().toInt();
}


void SettingsDialog::on_buttonBox_accepted()
{
    Settings s;
    s.general().queryConditions().limit(ui->limitLineEdit->text().toInt());
    s.general().view().followMode(ui->followmodeCheckBox->checkState() == Qt::Checked);
    QFont f = ui->fontComboBox->currentFont();
    f.setPointSize(ui->sizeSpin->value());
    s.general().view().font(f);
    s.general().view().fontSize(ui->sizeSpin->value());
    s.general().view().alternatingRowColors(ui->altRowColorsCheckBox->checkState() == Qt::Checked);
    //s.general().coloredColumns(ui->coloredColumnsSpinBox->value());
}
