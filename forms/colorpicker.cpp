#include "colorpicker.h"
#include "ui_colorpicker.h"
#include <qcolordialog.h>
#include <qmenu.h>
#include <qwidgetaction.h>
#include <qscreen.h>
ColorPicker::ColorPicker(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::ColorPicker())
{
	ui->setupUi(this);
	QTableWidget *tw = ui->tableAvailableColors;
	//tw->setRowCount(10);
	//tw->setColumnCount(10);
	//tw->setItem(8, 0, new QTableWidgetItem);
	//tw->item(8, 0)->setBackground(Qt::red);
	ColorList cl;
	for (int i = 0; i < 0; i++) {
		QColor color(i * 200);
		cl.push_back(color);
	}
	setAvailableColors(cl);
	connect(ui->tableAvailableColors, &QTableWidget::doubleClicked, this, [&](const QModelIndex& index) {
		auto idx = index.row() * maxColumns + index.column();
		if (idx >= _availableColors.size())
			return;
		QColor color = _availableColors.at(idx);
		_availableColors.erase(_availableColors.begin() + idx);// std::find_if(_availableColors.begin(), _availableColors.end(), [color](const QColor& _color) { return _color == color;  })
		_usedColors.push_back(color);
		setAvailableColors(_availableColors);
		setUsedColors(_usedColors);
		emit usedColorsChanged();
	});
	connect(ui->tableUsedColors, &QTableWidget::doubleClicked, this, [&](const QModelIndex& index) {
		auto idx = index.row() * maxColumns + index.column();
		if (idx >= _usedColors.size())
			return;
		QColor color = _usedColors.at(idx);
		_usedColors.erase(_usedColors.begin() + idx);
		_availableColors.push_back(color);
		setAvailableColors(_availableColors);
		setUsedColors(_usedColors);
		emit usedColorsChanged();
	});
	QColorDialog *d = new QColorDialog(this);
	d->setWindowFlags(Qt::Popup);
	d->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);

	auto action = new QWidgetAction(ui->btnAddColor);
	action->setDefaultWidget(d);
	auto menu = new QMenu(ui->btnAddColor);
	menu->addAction(action);
	ui->btnAddColor->setMenu(menu);


	connect(d, &QColorDialog::rejected, this, [=] { 
		menu->hide(); 
	});
	connect(menu, &QMenu::aboutToShow, this, [=]() {
		menu->show();
		d->show();
		d->setFocus();
	});
	disconnect(d, SIGNAL(accepted()), d, SLOT(accept()));
	connect(d, &QColorDialog::accepted, this, [=]() {
		QColor color;
		menu->hide();
		d->colorSelected(color);
		_availableColors.push_back(d->currentColor());
		setAvailableColors(_availableColors);
		emit availableColorsChanged();
	});
}

ColorPicker::~ColorPicker()
{

}

void ColorPicker::addColorsToTable(const ColorList& colors, QTableWidget *tw)
{
	tw->clear();
	int row = 0;
	int col = 0;
	int idx = 0;
	tw->setRowCount(colors.size() / maxColumns + 1);
	tw->setColumnCount(maxColumns);
	ColorList sortedColors = colors;
	foreach(QColor color, sortedColors)
	{
		tw->setItem(row, col, new QTableWidgetItem);
		tw->item(row, col)->setBackground(color);
		col = (col + 1) % maxColumns;
		if (col % maxColumns == 0)
			row++;
	}
}

void ColorPicker::setAvailableColors(const ColorList& colors)
{
	_availableColors = colors;
	std::sort(_availableColors.begin(), _availableColors.end(), [](const QColor& col1, const QColor& col2) {
		return col1.value() < col2.value();
	});
	addColorsToTable(_availableColors, ui->tableAvailableColors);
}

void ColorPicker::setUsedColors(const ColorList& colors)
{
	_usedColors = colors;
	std::sort(_usedColors.begin(), _usedColors.end(), [](const QColor& col1, const QColor& col2) {
		return col1.value() < col2.value();
	});
	addColorsToTable(_usedColors, ui->tableUsedColors);
}

const ColorList& ColorPicker::getAvailableColors() const
{
	return _availableColors;
}

const ColorList& ColorPicker::getUsedColors() const
{
	return _usedColors;
}