#include "generellwidget.h"
#include "ui_generellwidget.h"

GenerellWidget::GenerellWidget(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GenerellWidget)
{
    ui->setupUi(this);
}

GenerellWidget::~GenerellWidget()
{
    delete ui;
}
