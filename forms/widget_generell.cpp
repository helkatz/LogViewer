#include "generellwidget.h"
#include "ui_generellwidget.h"
#include "settings.h"
#include <logger/Logger.h>
GenerellWidget::GenerellWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GenerellWidget)
{
    ui->setupUi(this);
	Settings s;
	ui->limitLineEdit->setText(QString("%1").arg(s.general().queryConditions().limit()));
	ui->followmodeCheckBox->setChecked(s.general().view().followMode());
	ui->fontComboBox->setFont(s.general().view().font());
	ui->altRowColorsCheckBox->setChecked(s.general().view().alternatingRowColors());
	ui->fontSize->setValue(s.general().view().fontSize());
	auto list = logger::Logger::Level::toMap();
	for (auto item = list.begin(); item != list.end(); item++) {
		ui->logLevel->addItem(item->second.c_str());
	}
	ui->logLevel->setCurrentText(logger::Logger::Level(s.general().logLevel()).toString().c_str());
	ui->logFileEdit->setText(s.general().logFile());
}

GenerellWidget::~GenerellWidget()
{
    delete ui;
}

bool GenerellWidget::followMode() const
{
	return ui->followmodeCheckBox->checkState() == Qt::Checked;
}

int GenerellWidget::limit() const
{
	return ui->limitLineEdit->text().toInt();
}

void GenerellWidget::on_btnCancel_clicked()
{
	window()->close();
}

void GenerellWidget::on_logLevel_currentIndexChanged(int)
{
	logger::Logger::set_level(".*", logger::Logger::Level(ui->logLevel->currentText().toStdString()));
}

void GenerellWidget::on_btnSave_clicked()
{
	Settings s;
	s.general().queryConditions().limit(ui->limitLineEdit->text().toInt());
	s.general().view().followMode(ui->followmodeCheckBox->checkState() == Qt::Checked);
	QFont f = ui->fontComboBox->currentFont();
	f.setPointSize(ui->fontSize->value());
	s.general().view().font(f);
	s.general().view().fontSize(ui->fontSize->value());
	s.general().view().alternatingRowColors(ui->altRowColorsCheckBox->checkState() == Qt::Checked);
	s.general().logLevel(logger::Logger::Level(ui->logLevel->currentText().toStdString()));
	s.general().logFile(ui->logFileEdit->text());
	//s.general().coloredColumns(ui->coloredColumnsSpinBox->value());
}
