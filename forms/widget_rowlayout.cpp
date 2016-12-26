#include "rowlayoutwidget.h"
#include "ui_rowlayoutwidget.h"
#include <QSignalMapper>
#include <qwidgetaction.h>
#include <qmenu.h>
#include <qcolordialog.h>
#include "colorpicker.h"
#include "logview.h"
#include "settings.h"
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

	ui->spinColorizeCellByChar->setValue(rowStyle.getCellColorizer(_currentCell).colorizeByChars);
	connect(ui->spinColorizeCellByChar, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, [&](int colorizeByChars) {
		rowStyle.getCellColorizer(_currentCell).colorizeByChars = colorizeByChars;
		emit rowStyleChanged(_rowStyle);
	});

	ui->alternateCellColor->setChecked(rowStyle.getCellColorizer(_currentCell).alternateColors);
	connect(ui->alternateCellColor, &QCheckBox::toggled, this, [&](bool enabled) {
		rowStyle.getCellColorizer(_currentCell).alternateColors = enabled;
		emit rowStyleChanged(_rowStyle);
	});

	ui->spinColorizeRowByChar->setValue(rowStyle.getRowColorizer().colorizeByChars);
	connect(ui->spinColorizeRowByChar, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, [&](int colorizeByChars) {
		rowStyle.getRowColorizer().colorizeByChars = colorizeByChars;
		rowStyle.getRowColorizer().boundColumn = _currentCell;
		emit rowStyleChanged(_rowStyle);
	});

	foreach(auto part, rowStyle.textPartColorizer.getList())
	{
		ui->cbTextPartColor->addItem(part.textPart);
		ui->sliderTextPartColor->setValue(part.color.value());
	}

	ui->alternateRowColors->setChecked(rowStyle.alternateRowColors);
	connect(ui->alternateRowColors, &QCheckBox::toggled, this, [&](bool enabled) {
		rowStyle.alternateRowColors = enabled;
		emit rowStyleChanged(_rowStyle);
	});

	ui->cbFontStyle->setFont(rowStyle.font);
	connect(ui->cbFontStyle, &QFontComboBox::currentFontChanged, this, [&](const QFont& font) {
		rowStyle.font = font;
		emit rowStyleChanged(_rowStyle);
	});
	ui->spinFontSize->setValue(rowStyle.fontSize);
	connect(ui->spinFontSize, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, [&](int size) {
		rowStyle.fontSize = size;
		emit rowStyleChanged(_rowStyle);
	});
	/*
	rowLayoutWidget->getFontCombo()->setFont(font());
	connect(rowLayoutWidget->getFontCombo(), SIGNAL(currentFontChanged(QFont)), this, SLOT(setFont(QFont)));
	rowLayoutWidget->getFontSizeSpin()->setValue(font().pointSize());
	connect(rowLayoutWidget->getFontSizeSpin(), SIGNAL(valueChanged(int)), this, SLOT(setFontSize(int)));
	widgetAction = new QWidgetAction(&menu);
	*/
	addColorDialog(ui->btnPickCellColor);
	addColorDialog(ui->btnPickRowColor);
}
#if 0
RowLayoutWidget::RowLayoutWidget(QWidget *parent, LogView *logView, const QModelIndex& index) :
    QWidget(parent),
    ui(new Ui::RowLayoutWidget),
	logView(logView)
{
    //logView = qobject_cast<LogView *>(parent);
    ui->setupUi(this);

    // just called to initialize the sliders
    setTextPartColor(-1, -1, -1);

	//getAvailableCellColors(index.column()));
	//ui->spinLigthness->setValue(logView->getColorizeRow().ligthness);

//	rowLayoutWidget->setAvailableRowColors(getAvailableRowColors());
	//rowLayoutWidget->getColorizeColumnSpin()->setValue(getColorizeColumn(index.column()));
//	getLightnessSpin()->setValue(getColorizeRow().ligthness);
	
#if 1
	auto addColorDialog = [&](QToolButton *button) {

		ColorPicker *colorPicker = new ColorPicker(this);
		colorPicker->setWindowFlags(Qt::Widget);
		//colorPicker->hide();
		auto action = new QWidgetAction(button);
		action->setDefaultWidget(colorPicker);
		auto menu = new QMenu(button);
		menu->addAction(action);
		colorPicker->setAvailableColors(Settings().general().availableColors());

		connect(menu, &QMenu::aboutToShow, this, [=] {
			int idx = 0;
			if (button == ui->btnPickCellColor) {
				colorPicker->setUsedColors(logView->getAvailableCellColors(index.column()));
			}
			if (button == ui->btnPickRowColor) {
				colorPicker->setUsedColors(logView->getAvailableRowColors());
			}
			colorPicker->show();
		});
		connect(colorPicker, &ColorPicker::availableColorsChanged, this, [=] {
			Settings().general().availableColors(colorPicker->getAvailableColors());
		});
		connect(colorPicker, &ColorPicker::close, this, [=] { menu->hide();   });
		connect(colorPicker, &ColorPicker::usedColorsChanged, this, [=]() {
			//menu->hide();
			if (button == ui->btnPickCellColor) {
				logView->setAvailableCellColors(colorPicker->getUsedColors(), index.column());
			}
			//OnFillColorChanged(color); // Call the "slot" in this class
		});


		button->setMenu(menu);

		button->setPopupMode(QToolButton::InstantPopup);
		button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	};
	addColorDialog(ui->btnPickCellColor);
	addColorDialog(ui->btnPickRowColor);
#endif
}
#endif
void RowLayoutWidget::addColorDialog(QToolButton *button)
{
	ColorPicker *colorPicker = new ColorPicker(this);
	colorPicker->setWindowFlags(Qt::Widget);
	//colorPicker->hide();
	auto action = new QWidgetAction(button);
	action->setDefaultWidget(colorPicker);
	auto menu = new QMenu(button);
	menu->addAction(action);
	colorPicker->setAvailableColors(Settings().general().availableColors());

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
		Settings().general().availableColors(colorPicker->getAvailableColors());
	});
	connect(colorPicker, &ColorPicker::close, this, [=] { menu->hide();   });
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


	button->setMenu(menu);

	button->setPopupMode(QToolButton::InstantPopup);
	button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

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
    auto coloredTextPart = _rowStyle.textPartColorizer.getByText(text);
    if(!coloredTextPart) {
		_rowStyle.textPartColorizer.addText(text, QColor());
		coloredTextPart = _rowStyle.textPartColorizer.getByText(text);
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
	emit rowStyleChanged(_rowStyle);
}

void RowLayoutWidget::on_cbTextPartColor_currentIndexChanged(int index)
{
	Q_UNUSED(index);
    setTextPartColor(-1, -1, -1);
}

void RowLayoutWidget::on_sliderTextPartColor_valueChanged(int value)
{
    return;
    QString text = ui->cbTextPartColor->currentText();
    qDebug()<<"set color"<<value;
    QColor color(value);
    //QColor color(((double)value / ui->sliderTextPartColor->maximum()) * 0x00ffffff);
	_rowStyle.textPartColorizer.addText(text, color);
    emit rowStyleChanged(_rowStyle);
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

void RowLayoutWidget::xon_btnPickCellColor_clicked()
{
	QColorDialog *d = new QColorDialog(this);
	d->setWindowFlags(Qt::Widget);
	d->setOptions(
		/* do not use native dialog */
		QColorDialog::DontUseNativeDialog
		/* you don't need to set it, but if you don't set this
		the "OK" and "Cancel" buttons will show up, I don't
		think you'd want that. */
		| QColorDialog::NoButtons
		);

	auto widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(d);
	addAction(widgetAction);
	
	widgetAction->trigger();
	//this->parent()->blockSignals(true);
	//d->exec();
	//this->parent()->blockSignals(false);
}
void RowLayoutWidget::on_btnPickRowColor_clicked()
{
	QColorDialog *colorDialog = new QColorDialog(this);
	//QColorDialog d(this);
	//d.exec();
	colorDialog->setWindowFlags(Qt::Widget);
	/* a few options that we must set for it to work nicely */
	colorDialog->setOptions(
		/* do not use native dialog */
		QColorDialog::DontUseNativeDialog
		/* you don't need to set it, but if you don't set this
		the "OK" and "Cancel" buttons will show up, I don't
		think you'd want that. */
		| QColorDialog::NoButtons
		);
	colorDialog->exec();
}
