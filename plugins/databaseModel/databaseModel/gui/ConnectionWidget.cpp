#include <gui/ConnectionWidget.h>
#include <ui/ui_ConnectionWidget.h>
#include <QSqlDatabase>
#include <fmt/format.h>

#include "../DatabaseModelFactory.h"
#include "../Settings.h"
#include "../Helper.h"

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QDialog(parent)
	, ui(new Ui::ConnectionWidgetUi())
{
    ui->setupUi(this);

    QStringList drivers = QSqlDatabase::drivers();

    // remove compat names
    drivers.removeAll("QMYSQL3");
    drivers.removeAll("QOCI8");
    drivers.removeAll("QODBC3");
    drivers.removeAll("QPSQL7");
    drivers.removeAll("QTDS7");

    if (!drivers.contains("QSQLITE"))
        ui->dbCheckBox->setEnabled(false);

    ui->driverCombo->addItems(drivers);

    connect(parent, SIGNAL(saveSettings()), this, SLOT(saveSettings()));

    //loadSettings();

    //connect(newButton, &QPushButton::clicked, this, [this]() {
    //    getTabWidget()->newConnection();
    //    });

    auto onTextChanged = [this](QLineEdit *edit) {
        connect(edit, &QLineEdit::textChanged, this, [this]() {
            disableButtons(boost::none, boost::none, false);
        });
    };
    onTextChanged(ui->databaseEdit);
    onTextChanged(ui->hostEdit);
    //onTextChanged(ui->portSpinBox);
    onTextChanged(ui->passwordEdit);
}

ConnectionWidget::~ConnectionWidget()
{
}

void ConnectionWidget::disableButtons(boost::optional<bool> add
    , boost::optional<bool> del, boost::optional<bool> save)
{
    return;
}

QString ConnectionWidget::driverName() const
{
    return ui->driverCombo->currentText();
}

QString ConnectionWidget::databaseName() const
{
    return ui->databaseEdit->text();
}

QString ConnectionWidget::userName() const
{
    return ui->usernameEdit->text();
}

QString ConnectionWidget::password() const
{
    return ui->passwordEdit->text();
}

QString ConnectionWidget::hostName() const
{
    return ui->hostEdit->text();
}

int ConnectionWidget::port() const
{
    return ui->portSpinBox->value();
}


void ConnectionWidget::saveSettings()
{  
    QString name = ui->nameCombo->currentText();
    //ui->nameCombo->setEditable(false);
    DatabaseSettings settings;
    if (name.length() > 0) {
        auto s = settings.connections(name);
        s.driver(ui->driverCombo->currentText());
        s.host(ui->hostEdit->text());
        s.username(ui->usernameEdit->text());
        s.password(ui->passwordEdit->text());
    }
}

void ConnectionWidget::loadSettings()
{
    auto selectedName = ui->nameCombo->currentText();

    DatabaseSettings settings;
    ui->nameCombo->blockSignals(true);
    ui->nameCombo->clear();

    auto names = settings.connections()->childGroups().filter(QRegExp("(?!__cloned__db).*"));
    ui->nameCombo->addItems(names);

    if (selectedName.length() == 0 || ui->nameCombo->findText(selectedName) == -1) {
        ui->nameCombo->setCurrentIndex(0);
        selectedName = ui->nameCombo->currentText();
    }

    if (selectedName.length()) {
        auto s = settings.connections(selectedName);
        ui->nameCombo->setCurrentText(selectedName);
        ui->driverCombo->setCurrentText(s.driver());
        ui->hostEdit->setText(s.host());
        ui->databaseEdit->setText(s.database());
        ui->usernameEdit->setText(s.username());
        ui->passwordEdit->setText(s.password());
    }
    ui->nameCombo->blockSignals(false);
    //ui->cbName->setCurrentIndex(0);
}

bool ConnectionWidget::useInMemoryDatabase() const
{
    return ui->dbCheckBox->isChecked();
}

void ConnectionWidget::on_nameCombo_currentIndexChanged(const QString& text)
{
    log_trace(0) << "comboChanged" << text;
    disableButtons(true, false, true);
    loadSettings();
}

void ConnectionWidget::on_nameCombo_editTextChanged(const QString& text)
{
    disableButtons(false, true, true);
}

void ConnectionWidget::on_addButton_clicked()
{
    DatabaseSettings settings;
    QString name = ui->nameCombo->currentText();
    //ui->nameCombo->setEditable(false);
    if (name.length() > 0) {
        settings.connections(name).driver(ui->driverCombo->currentText());
        settings.connections(name).database(ui->databaseEdit->text());
        settings.connections(name).host(ui->hostEdit->text());
        settings.connections(name).username(ui->usernameEdit->text());
        settings.connections(name).password(ui->passwordEdit->text());
    }
    loadSettings();
}

void ConnectionWidget::on_saveButton_clicked()
{
}

void ConnectionWidget::on_deleteButton_clicked()
{
    DatabaseSettings settings;
    QString name = ui->nameCombo->currentText();
    settings.connections(name)->remove();
    loadSettings();
}
void ConnectionWidget::on_testButton_clicked()
{
    QString conName = "__temporary_for_test__";
    QSqlDatabase db = helper::getDatabase(conName,
        ui->driverCombo->currentText(),
        ui->hostEdit->text(),
        ui->databaseEdit->text(),
        ui->usernameEdit->text(),
        ui->passwordEdit->text());
    if (db.isValid() == true) {
        QMessageBox::information(this,
            "",
            QObject::tr("Connection Successful"));
    }

    QSqlDatabase::removeDatabase(conName);
}
void ConnectionWidget::on_dbCheckBox_clicked()
{
	ui->connGroupBox->setEnabled(!ui->dbCheckBox->isChecked()); 
}
