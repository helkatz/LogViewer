#include "connectionswidget.h"
#include "ui_connectionswidget.h"
#include "settings.h"
#include "Utils/utils.h"
#include <QSqlDatabase>
ConnectionsWidget::ConnectionsWidget(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionsWidget)
{
    ui->setupUi(this);
    QStringList l;
    l << "QSQLITE" << "QMYSQL";
    ui->cbDriver->addItems(l);
    ui->cbName->setVisible(true);
    loadSettings();
}

void ConnectionsWidget::saveSettings()
{
    Settings settings;
    QString name = ui->cbName->currentText();
    if(name.length() > 0) {
        settings.connections(name).driver(ui->cbDriver->currentText());
        settings.connections(name).database(ui->editDatabase->text());
        settings.connections(name).host(ui->editHost->text());
        settings.connections(name).username(ui->editUsername->text());
        settings.connections(name).password(ui->editPassword->text());
    }
}

void ConnectionsWidget::loadSettings()
{
    Settings settings;
    ui->cbName->clear();
    ui->cbName->addItems(settings.childGroups("connections").filter("(?!__cloned__db).*"));
    ui->cbName->addItems(settings.childGroups("connections"));
    //ui->cbName->setCurrentIndex(0);
}

ConnectionsWidget::~ConnectionsWidget()
{
    delete ui;
}

void ConnectionsWidget::on_btnSave_clicked()
{
    QString currentText = ui->cbName->currentText();
    saveSettings();
    loadSettings();
    ui->cbName->setCurrentIndex(ui->cbName->findText(currentText));
}

void ConnectionsWidget::on_btnCancel_clicked()
{
	window()->close();
}

void ConnectionsWidget::on_cbName_currentTextChanged(const QString &)
{
}

void ConnectionsWidget::on_cbName_currentIndexChanged(int)
{
    QString name = ui->cbName->currentText();
    if(name.length() == 0)
        return;
    Settings settings;
    ui->cbDriver->setCurrentText(settings.connections(name).driver());
    ui->editHost->setText(settings.connections(name).host());
    ui->editDatabase->setText(settings.connections(name).database());
    ui->editUsername->setText(settings.connections(name).username());
    ui->editPassword->setText(settings.connections(name).password());
}

void ConnectionsWidget::on_btnDelete_clicked()
{

    Settings settings;
    QString name = ui->cbName->currentText();
    settings.connections(name).remove();
    loadSettings();
}

void ConnectionsWidget::on_btnTest_clicked()
{
    ui->btnTest->setEnabled(false);
    QString conName = "__temporary_for_test__";
	QSqlDatabase db = utils::database::getDatabase(conName,
                       ui->cbDriver->currentText(),
                       ui->editHost->text(),
                       ui->editDatabase->text(),
                       ui->editUsername->text(),
                       ui->editPassword->text());
    if(db.isValid() == true) {
        QMessageBox::information(this,
            "",
            QObject::tr("Connection Successful"));
    }
    ui->btnTest->setEnabled(true);
    QSqlDatabase::removeDatabase(conName);
}
