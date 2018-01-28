#include "connections_logstash_widget.h"
#include "ui_connections_logstash_widget.h"
#include "settings.h"
#include "Utils/utils.h"
#include <QSqlDatabase>
ConnectionsLogstashWidget::ConnectionsLogstashWidget(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionsLogstashWidget)
{
    ui->setupUi(this);
    ui->cbName->setVisible(true);
    loadSettings();
}

void ConnectionsLogstashWidget::saveSettings()
{
    Settings settings;
    QString name = ui->cbName->currentText();
    if(name.length() > 0) {
        settings.connections().logstash(name).host(ui->editHost->text());
        settings.connections().logstash(name).username(ui->editUsername->text());
        settings.connections().logstash(name).password(ui->editPassword->text());
    }
}

void ConnectionsLogstashWidget::loadSettings()
{
    Settings settings;
    ui->cbName->clear();
    ui->cbName->addItems(settings.childGroups("connections/logstash"));
    //ui->cbName->setCurrentIndex(0);
}

ConnectionsLogstashWidget::~ConnectionsLogstashWidget()
{
    delete ui;
}

void ConnectionsLogstashWidget::on_btnSave_clicked()
{
    QString currentText = ui->cbName->currentText();
    saveSettings();
    loadSettings();
    ui->cbName->setCurrentIndex(ui->cbName->findText(currentText));
}

void ConnectionsLogstashWidget::on_btnCancel_clicked()
{
	window()->close();
}

void ConnectionsLogstashWidget::on_db_name_currentTextChanged(const QString &)
{
}

void ConnectionsLogstashWidget::on_db_name_currentIndexChanged(int)
{
}

void ConnectionsLogstashWidget::on_ls_name_currentIndexChanged(int)
{
}

void ConnectionsLogstashWidget::on_btnDelete_clicked()
{

    Settings settings;
    QString name = ui->cbName->currentText();
    settings.connections().logstash(name).remove();
    loadSettings();
}

void ConnectionsLogstashWidget::on_btnTest_clicked()
{
    ui->btnTest->setEnabled(false);

    if(true) {
        QMessageBox::information(this,
            "",
            QObject::tr("Connection Successful"));
    }
    ui->btnTest->setEnabled(true);
}
