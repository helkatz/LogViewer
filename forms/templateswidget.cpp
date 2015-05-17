#include "templateswidget.h"
#include "ui_templateswidget.h"
#include "../settings.h"
#include "../mainwindow.h"
#include "../logview.h"
TemplatesWidget::TemplatesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemplatesWidget)
{
    ui->setupUi(this);
    foreach (QMdiSubWindow *frame, MainWindow::instance().getArea().subWindowList()) {
        LogWindow *logView = qobject_cast<LogWindow *>(frame->widget());
        QVariant userData(QMetaType::QObjectStar, &logView);
        if(logView) {
            ui->cbTemplateWindow->addItem(logView->windowTitle(), userData);
        }
    }
    loadSettings();
}

void TemplatesWidget::saveSettings()
{
    Settings settings;
    QString name = ui->cbName->currentText();
    if(name.length() > 0) {
        settings.windowTemplates(name).tableNameFilter(ui->editTableFilter->text());
        settings.windowTemplates(name).viewNameFilter(ui->editViewNameFilter->text());
        QVariant q = ui->cbTemplateWindow->itemData(ui->cbTemplateWindow->currentIndex());
        if(q.canConvert(QMetaType::QObjectStar)) {
            LogWindow *window = q.value<LogWindow *>();
            // write all settings from the template view to the template path
            window->writeSettings(settings.windowTemplates().logView().getPath());
            // remove unnecessary settings
            //settings.windowTemplates().logView()..queryConditions().remove();
        }
    }
}

void TemplatesWidget::loadSettings()
{
    Settings settings;
    ui->cbName->clear();
    ui->cbName->addItems(settings.childGroups("windowTemplates"));
}
TemplatesWidget::~TemplatesWidget()
{
    delete ui;
}

void TemplatesWidget::on_cbName_currentIndexChanged(const QString &arg1)
{
    QString name = ui->cbName->currentText();
    if(name.length() == 0)
        return;
    Settings settings;
    ui->editViewNameFilter->setText(settings.windowTemplates(name).viewNameFilter());
    ui->editTableFilter->setText(settings.windowTemplates(name).tableNameFilter());
}

void TemplatesWidget::on_cbName_currentTextChanged(const QString &arg1)
{

}

void TemplatesWidget::on_btnSave_clicked()
{
    QString currentText = ui->cbName->currentText();
    saveSettings();
    loadSettings();
    ui->cbName->setCurrentIndex(ui->cbName->findText(currentText));
}

void TemplatesWidget::on_btnDelete_clicked()
{

}

void TemplatesWidget::on_btnCancel_clicked()
{

}
