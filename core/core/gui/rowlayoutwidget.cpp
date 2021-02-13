#include <gui/rowlayoutwidget.h>
#include <ui/ui_rowlayoutwidget.h>

#include <gui/colorpicker.h>
#include <gui/logview/LogView.h>

#include <core/settings.h>


#include <QSignalMapper>
#include <qstandarditemmodel.h>
#include <qtreeview.h>
#include <qwidgetaction.h>
#include <qmenu.h>
#include <qcolordialog.h>
//#include "qtcolorpicker.h"

RowLayoutWidget::RowLayoutWidget(QWidget *parent, RowStyle& rowStyle, const QModelIndex& index):
	QWidget(parent),
	ui(new Ui::RowLayoutWidget),
	_rowStyle(rowStyle),
	_currentCell(index.column())
{
	ui->setupUi(this);
	addColorDialog(ui->btnPickCellColor);
	addColorDialog(ui->btnPickRowColor);

	ui->spinColorizeCellByChar->setValue(_rowStyle.getCellColorizer(_currentCell).colorizeByChars);
	connect(ui->spinColorizeCellByChar, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, [&](int colorizeByChars) {
		_rowStyle.getCellColorizer(_currentCell).colorizeByChars = colorizeByChars;
		emit rowStyleChanged(_rowStyle);
	});

	ui->spinColorizeCellByChar->setValue(_rowStyle.getCellColorizer(_currentCell).colorizeByChars);
	connect(ui->spinColorizeCellByChar, (void (QSpinBox::*)(int)) & QSpinBox::valueChanged, this, [&](int colorizeByChars) {
		_rowStyle.getCellColorizer(_currentCell).colorizeByChars = colorizeByChars;
		emit rowStyleChanged(_rowStyle);
		});

	ui->alternateCellColor->setChecked(_rowStyle.getCellColorizer(_currentCell).alternateColors);
	connect(ui->alternateCellColor, &QCheckBox::toggled, this, [&](bool enabled) {
		_rowStyle.getCellColorizer(_currentCell).alternateColors = enabled;
		emit rowStyleChanged(_rowStyle);
	});

	ui->spinColorizeRowByChar->setValue(_rowStyle.getRowColorizer().colorizeByChars);
	connect(ui->spinColorizeRowByChar, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, [&](int colorizeByChars) {
		_rowStyle.getRowColorizer().colorizeByChars = colorizeByChars;
		_rowStyle.getRowColorizer().boundColumn = _currentCell;
		emit rowStyleChanged(_rowStyle);
	});

	ui->textColorizer->initialize(rowStyle.textColorizer.getList());
	connect(ui->textColorizer, &TextColorizerWidget::itemsChanged, this, [&](const TextColorize::List& items) {
		_rowStyle.textColorizer.set(items);
		emit rowStyleChanged(_rowStyle);
	});
	connect(ui->textColorizer, &TextColorizerWidget::itemChanged, this, [&](const TextColorize& item) {
		_rowStyle.textColorizer.add(item);
		emit rowStyleChanged(_rowStyle);
	});

	ui->alternateRowColors->setChecked(_rowStyle.alternateRowColors);
	connect(ui->alternateRowColors, &QCheckBox::toggled, this, [&](bool enabled) {
		_rowStyle.alternateRowColors = enabled;
		emit rowStyleChanged(_rowStyle);
	});

	ui->cbFontStyle->setFont(_rowStyle.font);
	connect(ui->cbFontStyle, &QFontComboBox::currentFontChanged, this, [&](const QFont& font) {
		_rowStyle.font = font;
		emit rowStyleChanged(_rowStyle);
	});

	ui->spinFontSize->setValue(_rowStyle.font.pointSize());
	connect(ui->spinFontSize, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, [&](int size) {
		_rowStyle.font.setPointSize(size);
		emit rowStyleChanged(_rowStyle);
	});
}

void RowLayoutWidget::addColorDialog(QToolButton *button)
{
	ColorPicker *colorPicker = new ColorPicker(this);
	colorPicker->setWindowFlags(Qt::Widget);
	//colorPicker->hide();
	auto action = new QWidgetAction(button);
	action->setDefaultWidget(colorPicker);
	auto menu = new QMenu(button);
	menu->addAction(action);
	colorPicker->setAvailableColors(appSettings().general().availableColors());
	
	button->setMenu(menu);

	connect(menu, &QMenu::aboutToShow, this, [=] {
		int idx = 0;
		if (button == ui->btnPickCellColor) {
			colorPicker->setUsedColors(_rowStyle.getCellColorizer(_currentCell).availableColors);
		}
		if (button == ui->btnPickRowColor) {
			colorPicker->setUsedColors(_rowStyle.getRowColorizer().availableColors);
		}
		colorPicker->show();
	});

	connect(colorPicker, &ColorPicker::availableColorsChanged, this, [=] {
		appSettings().general().availableColors(colorPicker->getAvailableColors());
	});

	connect(colorPicker, &ColorPicker::close, this, [=] { 
		menu->hide();   
	});

	connect(colorPicker, &ColorPicker::usedColorsChanged, this, [=]() {
		//menu->hide();
		if (button == ui->btnPickCellColor) {
			_rowStyle.getCellColorizer(_currentCell).availableColors = colorPicker->getUsedColors();
		}
		if (button == ui->btnPickRowColor) {
			_rowStyle.getRowColorizer().availableColors = colorPicker->getUsedColors();
		}
		emit rowStyleChanged(_rowStyle);
		//OnFillColorChanged(color); // Call the "slot" in this class
	});
}

RowLayoutWidget::~RowLayoutWidget()
{
    delete ui;
}

QSpinBox *RowLayoutWidget::getColorizeColumnSpin()
{
    return ui->spinColorizeCellByChar;
}

bool RowLayoutWidget::isColorizeFullRow()
{
	return false; //ui->colorizeFullRow->isChecked();
}

QSpinBox *RowLayoutWidget::getColorizeRowSpin()
{
    return ui->spinColorizeRowByChar;
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
    return ui->alternateRowColors;
}

void RowLayoutWidget::on_btnPickRowColor_clicked()
{
	QColorDialog *colorDialog = new QColorDialog(this);

	colorDialog->setWindowFlags(Qt::Widget);
	colorDialog->setOptions(
		QColorDialog::DontUseNativeDialog
		| QColorDialog::NoButtons
		);
	colorDialog->exec();
}
