#include <core/settings.h>
#include <gui/settings_templates.h>
#include <ui/ui_settings_templates.h>

#include <qmdisubwindow.h>
TemplatesWidget::TemplatesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TemplatesWidget)
{
    ui->setupUi(this);

    connect(parent, SIGNAL(saveSettings()), this, SLOT(saveSettings()));
//@TODO rework
#if 0
    foreach (QMdiSubWindow *frame, MainWindow::instance().getArea().subWindowList()) {
        LogWindow *logView = qobject_cast<LogWindow *>(frame->widget());
        QVariant userData(QMetaType::QObjectStar, &logView);
        if(logView) {
            ui->cbTemplateWindow->addItem(logView->windowTitle(), userData);
        }
    }
#endif
    loadSettings();
}

TemplatesWidget::~TemplatesWidget()
{
    delete ui;
}

void TemplatesWidget::saveSettings()
{
    QString selectedName = ui->nameCombo->currentText();
    if(selectedName.length() > 0) {
        auto s = appSettings().logWindowTemplates(selectedName);
        s.tableNameFilter(ui->editTableFilter->text());
        s.viewNameFilter(ui->editViewNameFilter->text());
        QVariant q = ui->cbTemplateWindow->itemData(ui->cbTemplateWindow->currentIndex());
#if 0
        if(q.canConvert(QMetaType::QObjectStar)) {
            LogWindow *window = q.value<LogWindow *>();
            // write all settings from the template view to the template path
            window->writeSettings(settings.windowTemplates().logView().getPath());
            // remove unnecessary settings
            //settings.windowTemplates().logView()..QueryOptions().remove();
        }
#endif
    }

}

void TemplatesWidget::loadSettings()
{
    ui->nameCombo->blockSignals(true);
    QString selectedName = ui->nameCombo->currentText();
    ui->nameCombo->clear();
    ui->nameCombo->addItems(appSettings().logWindowTemplates().childGroups());

    if (selectedName.length()) {
        auto s = appSettings().logWindowTemplates(selectedName);
        ui->nameCombo->setCurrentText(selectedName);
        ui->editViewNameFilter->setText(s.viewNameFilter());
        ui->editTableFilter->setText(s.tableNameFilter());
    }
}

void TemplatesWidget::on_cbName_currentIndexChanged(const QString &)
{
    QString name = ui->nameCombo->currentText();
    if(name.length() == 0)
        return;
    auto s = appSettings().logWindowTemplates(name);
    ui->editViewNameFilter->setText(s.viewNameFilter());
    ui->editTableFilter->setText(s.tableNameFilter());
}

void TemplatesWidget::on_cbName_currentTextChanged(const QString &)
{

}

void TemplatesWidget::on_btnSave_clicked()
{
    //QString currentText = ui->nameCombo->currentText();
    //saveSettings();
    //loadSettings();
    //ui->nameCombo->setCurrentIndex(ui->nameCombo->findText(currentText));
}

void TemplatesWidget::on_btnDelete_clicked()
{

}

void TemplatesWidget::on_btnCancel_clicked()
{
	window()->close();
}
