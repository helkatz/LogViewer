#ifndef ROWLAYOUTWIDGET_H
#define ROWLAYOUTWIDGET_H

#include <QWidget>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include "../logview.h"
namespace Ui {
class RowLayoutWidget;
}

class RowLayoutWidget : public QWidget
{
    Q_OBJECT
private:
    Ui::RowLayoutWidget *ui;
    LogView *logView;
public:
    explicit RowLayoutWidget(QWidget *parent = 0);
    ~RowLayoutWidget();

    QSpinBox *getColorizeColumnSpin();
    QSpinBox *getColorizeRowSpin();
    QFontComboBox *getFontCombo();
    QSpinBox *getFontSizeSpin();
    QSpinBox *getLightnessSpin();
    QCheckBox *getAlternateRowColorCheck();
    QComboBox *getTextPartColorCombo();
    QSlider *getTextPartColorSlider();

private slots:
    void setTextPartColor(int red, int green, int blue);
    void on_cbTextPartColor_currentIndexChanged(int index);
    void on_sliderTextPartColor_valueChanged(int value);

    void on_sliderRed_valueChanged(int value);

    void on_sliderGreen_valueChanged(int value);

    void on_sliderBlue_valueChanged(int value);

signals:
    void currentFontSizeChanged(int);
};

#endif // ROWLAYOUTWIDGET_H
