#include "connectionswidget.h"
#include "ui_connectionswidget.h"
#include "settings.h"
#include "Utils/utils.h"
#include <QSqlDatabase>
#include <qfileinfo.h>
ConnectionsWidget::ConnectionsWidget(QDialog *parent) :
    QDialog(parent)
{
	tabWidget = new QTabWidget;

	databaseTab = new DatabaseTab();
	logstashTab = new LogstashTab();
	tabWidget->addTab(databaseTab, tr("Database"));
	tabWidget->addTab(logstashTab, tr("Logstash"));

	auto buttonLayout = new QHBoxLayout();

	auto newButton = new QPushButton(tr("New"));
	buttonLayout->addWidget(newButton);

	auto saveButton = new QPushButton(tr("Save"));	
	buttonLayout->addWidget(saveButton);

	auto deleteButton = new QPushButton(tr("Delete"));
	buttonLayout->addWidget(deleteButton);

	auto testButton = new QPushButton(tr("Test"));	
	buttonLayout->addWidget(testButton);
	
	buttonLayout->addStretch(10);

	auto cancelButton = new QPushButton(tr("Cancel"));	
	buttonLayout->addWidget(cancelButton);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addItem(buttonLayout);
	setLayout(mainLayout);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	connect(newButton, &QPushButton::clicked, this, [this]() {
		getTabWidget()->newConnection();
	});

	connect(testButton, &QPushButton::clicked, this, [this]() {
		getTabWidget()->testConnection();
	});

	connect(saveButton, &QPushButton::clicked, this, [this]() {
		saveSettings();
		loadSettings();
	});

	connect(deleteButton, &QPushButton::clicked, this, [this]() {
		getTabWidget()->deleteConnection();
	});
}

AbstractTab *ConnectionsWidget::getTabWidget()
{
	return dynamic_cast<AbstractTab *>(tabWidget->widget(tabWidget->currentIndex()));
};

void ConnectionsWidget::saveSettings()
{
	databaseTab->saveSettings();
	logstashTab->saveSettings();
}

void ConnectionsWidget::loadSettings()
{
	databaseTab->loadSettings();
	logstashTab->loadSettings();
}

ConnectionsWidget::~ConnectionsWidget()
{
}

AbstractTab::AbstractTab(QWidget *parent)
	: QWidget(parent)
{
}

DatabaseTab::DatabaseTab(QWidget *parent)
	: AbstractTab(parent)
{
	nameCombo = new QComboBox();	
	driverCombo = new QComboBox();
	hostEdit = new QLineEdit();
	databaseEdit = new QLineEdit();
	usernameEdit = new QLineEdit();
	passwordEdit = new QLineEdit();

	auto formLayout = new QFormLayout();
	formLayout->addRow(new QLabel(tr("Name")), nameCombo);
	formLayout->addRow(new QLabel(tr("Driver")), driverCombo);
	formLayout->addRow(new QLabel(tr("Host")), hostEdit);
	formLayout->addRow(new QLabel(tr("Database")), databaseEdit);
	formLayout->addRow(new QLabel(tr("Username")), usernameEdit);
	formLayout->addRow(new QLabel(tr("Password")), passwordEdit);
	auto gridLayout = new QGridLayout();
	gridLayout->addItem(formLayout, 0, 0);
	gridLayout->addItem(new QSpacerItem(10, 10), 0, 1);

	setLayout(gridLayout);

	QStringList l;
	l << "QSQLITE" << "QMYSQL";
	driverCombo->addItems(l);
	loadSettings();

	connect(nameCombo, &QComboBox::currentTextChanged, this, [this]() {
		loadSettings();
	});
}

void DatabaseTab::testConnection()
{
	//ui->btnTest->setEnabled(false);
	QString conName = "__temporary_for_test__";
	QSqlDatabase db = utils::database::getDatabase(conName,
		driverCombo->currentText(),
		hostEdit->text(),
		databaseEdit->text(),
		usernameEdit->text(),
		passwordEdit->text());
	if (db.isValid() == true) {
		QMessageBox::information(this,
			"",
			QObject::tr("Connection Successful"));
	}
	//ui->btnTest->setEnabled(true);
	QSqlDatabase::removeDatabase(conName);
}

void DatabaseTab::deleteConnection()
{
	Settings settings;
	QString name = nameCombo->currentText();
	settings.connections().database(name).remove();
	loadSettings();
}

void DatabaseTab::newConnection()
{
	nameCombo->setEditable(true);
	nameCombo->blockSignals(true);
}

void DatabaseTab::saveSettings()
{
	Settings settings;
	QString name = nameCombo->currentText();
	nameCombo->setEditable(false);
	if (name.length() > 0) {
		settings.connections().database(name).driver(driverCombo->currentText());
		settings.connections().database(name).database(databaseEdit->text());
		settings.connections().database(name).host(hostEdit->text());
		settings.connections().database(name).username(usernameEdit->text());
		settings.connections().database(name).password(passwordEdit->text());
	}
}

void DatabaseTab::loadSettings()
{
	Settings settings;
	auto name = nameCombo->currentText();
	nameCombo->blockSignals(true);
	nameCombo->clear();
	nameCombo->addItems(settings.childGroups("connections/database").filter("(?!__cloned__db).*"));
	nameCombo->addItems(settings.childGroups("connections/database"));
	if (name.length() == 0 || nameCombo->findText(name) == -1)
		name = nameCombo->currentText();

	if (name.length()) {
		nameCombo->setCurrentText(name);
		driverCombo->setCurrentText(settings.connections().database(name).driver());
		hostEdit->setText(settings.connections().database(name).host());
		databaseEdit->setText(settings.connections().database(name).database());
		usernameEdit->setText(settings.connections().database(name).username());
		passwordEdit->setText(settings.connections().database(name).password());
	}
	nameCombo->blockSignals(false);
	//ui->cbName->setCurrentIndex(0);
}


LogstashTab::LogstashTab(QWidget *parent)
	: AbstractTab(parent)
{
	nameCombo = new QComboBox();
	hostEdit = new QLineEdit();
	usernameEdit = new QLineEdit();
	passwordEdit = new QLineEdit();

	auto formLayout = new QFormLayout();
	formLayout->addRow(new QLabel(tr("Name")), nameCombo);
	formLayout->addRow(new QLabel(tr("Host")), hostEdit);
	formLayout->addRow(new QLabel(tr("Username")), usernameEdit);
	formLayout->addRow(new QLabel(tr("Password")), passwordEdit);
	auto gridLayout = new QGridLayout();
	gridLayout->addItem(formLayout, 0, 0);
	gridLayout->addItem(new QSpacerItem(10, 10), 0, 1);
	setLayout(gridLayout);
	loadSettings();

	connect(nameCombo, &QComboBox::currentTextChanged, this, [this]() {
		loadSettings();
	});
}

void LogstashTab::testConnection()
{

}

void LogstashTab::deleteConnection()
{
	Settings settings;
	QString name = nameCombo->currentText();
	settings.connections().logstash(name).remove();
	loadSettings();
}

void LogstashTab::newConnection()
{
	nameCombo->setEditable(true);
	nameCombo->blockSignals(true);
}

void LogstashTab::saveSettings()
{
	Settings settings;
	QString name = nameCombo->currentText();
	nameCombo->setEditable(false);
	if (name.length() > 0) {
		settings.connections().logstash(name).host(hostEdit->text());
		settings.connections().logstash(name).username(usernameEdit->text());
		settings.connections().logstash(name).password(passwordEdit->text());
	}
}

void LogstashTab::loadSettings()
{
	Settings settings;
	auto name = nameCombo->currentText();
	nameCombo->blockSignals(true);
	nameCombo->clear();
	nameCombo->addItems(settings.childGroups("connections/logstash"));

	if (name.length() == 0 || nameCombo->findText(name) == -1)
		name = nameCombo->currentText();

	if (name.length()) {
		nameCombo->setCurrentText(name);
		hostEdit->setText(settings.connections().logstash(name).host());
		usernameEdit->setText(settings.connections().logstash(name).username());
		passwordEdit->setText(settings.connections().logstash(name).password());
	}
	nameCombo->blockSignals(false);
	//ui->cbName->setCurrentIndex(0);
}