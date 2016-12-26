#include "fontstylewidget.h"
#include "ui_fontstylewidget.h"

FontStyleWidget::FontStyleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FontStyleWidget)
{
    ui->setupUi(this);
}

FontStyleWidget::~FontStyleWidget()
{
    delete ui;
}

QFontComboBox *FontStyleWidget::getFontCombo()
{
    return ui->cbFontStyle;
}

QSpinBox *FontStyleWidget::getFontSizeSpin()
{
    return ui->spinFontSize;
}
