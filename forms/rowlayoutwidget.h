#ifndef ROWLAYOUTWIDGET_H
#define ROWLAYOUTWIDGET_H

#include <QWidget>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include "logview.h"
namespace Ui {
class RowLayoutWidget;
}

class RowLayoutWidget : public QWidget
{
    Q_OBJECT
private:
    Ui::RowLayoutWidget *ui;
    LogView *logView;
	RowStyle& _rowStyle;
	int _currentCell;
	void addColorDialog(QToolButton *button);
public:
	
	explicit RowLayoutWidget(QWidget *parent, RowStyle& rowStyle, const QModelIndex& index);

    explicit RowLayoutWidget(QWidget *parent, LogView *logView, const QModelIndex& index);
    ~RowLayoutWidget();

    QSpinBox *getColorizeColumnSpin();
	bool isColorizeFullRow();
    QSpinBox *getColorizeRowSpin();
    QFontComboBox *getFontCombo();
    QSpinBox *getFontSizeSpin();
    QSpinBox *getLightnessSpin();
    QCheckBox *getAlternateRowColorCheck();
    QComboBox *getTextPartColorCombo();
    QSlider *getTextPartColorSlider();
	void setAvailableCellColors(const ColorList& colors);
	void setAvailableRowColors(const ColorList& colors);
	const ColorList& getAvailableCellColors() const;
	const ColorList& getAvailableRowColors() const;
private slots:
    void setTextPartColor(int red, int green, int blue);
    void on_cbTextPartColor_currentIndexChanged(int index);
    void on_sliderTextPartColor_valueChanged(int value);

    void on_sliderRed_valueChanged(int value);

    void on_sliderGreen_valueChanged(int value);

    void on_sliderBlue_valueChanged(int value);

	void xon_btnPickCellColor_clicked();
	void on_btnPickRowColor_clicked();
signals:
    void currentFontSizeChanged(int);
	void rowStyleChanged(const RowStyle& rowStyle);
};

#endif // ROWLAYOUTWIDGET_H
