#include <gui/settings_generell.h>
//#include <ui/ui_generell.h>
#include <ui/ui_settings_generell.h>
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

bool GenerellWidget::followMode() const
{
	return ui->followmodeCheckBox->checkState() == Qt::Checked;
}

int GenerellWidget::limit() const
{
	return ui->limitLineEdit->text().toInt();
}

void GenerellWidget::saveSettings()
{
	//appSettings().general().queryConditions().limit(ui->limitLineEdit->text().toInt());
	//appSettings().general().view().followMode(ui->followmodeCheckBox->checkState() == Qt::Checked);
	QFont f = ui->fontComboBox->currentFont();
	f.setPointSize(ui->fontSize->value());
	//appSettings().general().font(f);
	//appSettings().general().view().fontSize(ui->fontSize->value());
	//appSettings().general().view().alternatingRowColors(ui->altRowColorsCheckBox->checkState() == Qt::Checked);
	appSettings().general().logLevel(logger::Logger::Level(ui->logLevel->currentText().toStdString()));
	appSettings().general().logFile(ui->logFileEdit->text());
}

void GenerellWidget::loadSettings()
{
	//ui->limitLineEdit->setText(QString("%1").arg(appSettings().general().queryConditions().limit()));
	//ui->followmodeCheckBox->setChecked(appSettings().general().view().followMode());
	//ui->fontComboBox->setFont(appSettings().general().view().font());
	//ui->altRowColorsCheckBox->setChecked(appSettings().general().view().alternatingRowColors());
	//ui->fontSize->setValue(appSettings().general().view().fontSize());
	auto list = logger::Logger::Level::toMap();
	for (auto item = list.begin(); item != list.end(); item++) {
		ui->logLevel->addItem(item->second.c_str());
	}
	ui->logLevel->setCurrentText(logger::Logger::Level(appSettings().general().logLevel()).toString().c_str());
	ui->logFileEdit->setText(appSettings().general().logFile());
}

void GenerellWidget::on_logLevel_currentIndexChanged(int)
{
	logger::Logger::set_level(".*", logger::Logger::Level(ui->logLevel->currentText().toStdString()));
}

