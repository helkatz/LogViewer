#include "rowlayoutwidget.h"
#include "ui_rowlayoutwidget.h"
#include "qtcolorpicker.h"

RowLayoutWidget::RowLayoutWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RowLayoutWidget)
{
    logView = qobject_cast<LogView *>(parent);
    ui->setupUi(this);

    // just called to initialize the sliders
    setTextPartColor(-1, -1, -1);

    //on_cbTextPartColor_currentIndexChanged(ui->cbTextPartColor->currentIndex());
}

RowLayoutWidget::~RowLayoutWidget()
{
    delete ui;
}

QSpinBox *RowLayoutWidget::getColorizeColumnSpin()
{
    return ui->spinColorizeCell;
}

QSpinBox *RowLayoutWidget::getColorizeRowSpin()
{
    return ui->spinColorizeRow;
}

QFontComboBox *RowLayoutWidget::getFontCombo()
{
    return ui->cbFontStyle;
}

QSpinBox *RowLayoutWidget::getFontSizeSpin()
{
    return ui->spinFontSize;
}

QSpinBox *RowLayoutWidget::getLightnessSpin()
{
   return ui->spinLigthness;
}

QCheckBox *RowLayoutWidget::getAlternateRowColorCheck()
{
    return ui->checkBox;
}

QComboBox *RowLayoutWidget::getTextPartColorCombo()
{
    return ui->cbTextPartColor;
}

QSlider *RowLayoutWidget::getTextPartColorSlider()
{
    return ui->sliderTextPartColor;
}

void RowLayoutWidget::setTextPartColor(int red, int green, int blue)
{
    qDebug()<<"setTextPartColor("<<red<<","<<green<<","<<blue<<")";
    QString text = ui->cbTextPartColor->currentText();
    auto coloredTextPart = logView->getColorizer().getByText(text);
    if(!coloredTextPart) {
        logView->getColorizer().addText(text, QColor());
        coloredTextPart = logView->getColorizer().getByText(text);
    }

    if(red >= 0)
        coloredTextPart->color.setRed(red);
    if(green >= 0)
        coloredTextPart->color.setGreen(green);
    if(blue >= 0)
        coloredTextPart->color.setBlue(blue);
    //QColor color(((double)value / ui->sliderTextPartColor->maximum()) * 0x00ffffff);
    if(red == -1 && green == -1 && blue == -1) {
        ui->sliderRed->setValue(coloredTextPart->color.red());
        ui->sliderGreen->setValue(coloredTextPart->color.green());
        ui->sliderBlue->setValue(coloredTextPart->color.blue());
    }
    emit logView->model()->layoutChanged();
}

void RowLayoutWidget::on_cbTextPartColor_currentIndexChanged(int index)
{
    setTextPartColor(-1, -1, -1);
    //const TColoredTextParts& parts = logView->getColorizer().getList();
    return;
    QString text = ui->cbTextPartColor->currentText();
    auto coloredTextPart = logView->getColorizer().getByText(text);
    if(coloredTextPart) {
        int v = coloredTextPart->color.rgb() & 0x00ffffff;
//int value = (v / 0x00ffffff) * ui->sliderTextPartColor->maximum();
        qDebug() << "set slider pos "<<v;
        //ui->sliderTextPartColor->setValue(v);
    }
}

void RowLayoutWidget::on_sliderTextPartColor_valueChanged(int value)
{
    return;
    QString text = ui->cbTextPartColor->currentText();
    qDebug()<<"set color"<<value;
    QColor color(value);
    //QColor color(((double)value / ui->sliderTextPartColor->maximum()) * 0x00ffffff);
    logView->getColorizer().addText(text, color);
    emit logView->model()->layoutChanged();
}

void RowLayoutWidget::on_sliderRed_valueChanged(int value)
{
    setTextPartColor(value, -1, -1);
}

void RowLayoutWidget::on_sliderGreen_valueChanged(int value)
{
    setTextPartColor(-1, value, -1);
}

void RowLayoutWidget::on_sliderBlue_valueChanged(int value)
{
    setTextPartColor(-1, -1, value);
}
