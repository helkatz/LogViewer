#ifndef FONTSTYLEWIDGET_H
#define FONTSTYLEWIDGET_H

#include <QWidget>
#include <QFontComboBox>
#include <QSpinBox>
namespace Ui {
class FontStyleWidget;
}

class FontStyleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FontStyleWidget(QWidget *parent = 0);
    ~FontStyleWidget();

    QFontComboBox *getFontCombo();
    QSpinBox *getFontSizeSpin();
private:
    Ui::FontStyleWidget *ui;
};

#endif // FONTSTYLEWIDGET_H
