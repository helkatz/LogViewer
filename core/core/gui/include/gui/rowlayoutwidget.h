#ifndef ROWLAYOUTWIDGET_H
#define ROWLAYOUTWIDGET_H
#include <gui/logview/LogView.h>

#include <QWidget>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>

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
	void setAvailableCellColors(const types::ColorList& colors);
	void setAvailableRowColors(const types::ColorList& colors);
	const types::ColorList& getAvailableCellColors() const;
	const types::ColorList& getAvailableRowColors() const;
private slots:

	void on_btnPickRowColor_clicked();
signals:
    void currentFontSizeChanged(int);
	void rowStyleChanged(const RowStyle& rowStyle);
};

#endif // ROWLAYOUTWIDGET_H
