#include "ColorPickerWidget.h"
#include "ui_colorpickerwidget.h"
#include <qcolordialog.h>
#include <qmenu.h>
#include <qwidgetaction.h>
#include <qscreen.h>

ColorTableWidget::ColorTableWidget(QWidget *parent)
	: QTableWidget(parent)
{
	setSelectionMode(QAbstractItemView::NoSelection);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	connect(this, &QTableWidget::doubleClicked, this, [&](const QModelIndex& index) {
		size_t idx = index.row() * maxColumns + index.column();
		if (idx >= _colors.size())
			return;
		QColor color = _colors.at(idx);
		_colors.erase(_colors.begin() + idx);
		addColors(_colors);
		emit colorsChanged();
	});


	connect(this, &QTableWidget::pressed, this, [&](const QModelIndex &index) {
		auto it = item(index.row(), index.column());
		if (it == nullptr)
			return;
		auto buttons = QApplication::mouseButtons();
		if (buttons == Qt::MouseButton::LeftButton)
			emit foregroundColorChanged(item(index.row(), index.column())->backgroundColor());
		if (buttons == Qt::MouseButton::RightButton)
			emit backgroundColorChanged(item(index.row(), index.column())->backgroundColor());
	});
}

ColorTableWidget::~ColorTableWidget()
{

}

void ColorTableWidget::resizeEvent(QResizeEvent *event)
{
	resizeColumnsToContents();
}

void ColorTableWidget::addColors(const ColorList& colors)
{
	_colors.append(colors);

	//@TODO find a right colors sorting
	// https://www.alanzucconi.com/2015/09/30/colour-sorting/
	std::sort(_colors.begin(), _colors.end(), [](const QColor& col1, const QColor& col2) {
		if (col1.red() != col2.red())
			return col1.red() < col2.red();
		if (col1.green() != col2.green())
			return col1.green() < col2.green();
		if (col1.blue() != col2.blue())
			return col1.blue() < col2.blue();


		return col1.hsvHueF() < col2.hsvHueF();
		return sqrt(.241 * col1.red() + .691 * col1.green() + .068 * col1.blue())
			< sqrt(.241 * col2.red() + .691 * col2.green() + .068 * col2.blue());
		
		
	});
	//std::sort(_colors.begin(), _colors.end());
	_colors.erase(std::unique(_colors.begin(), _colors.end()), _colors.end());

	clear();
	setRowCount(_colors.size() / maxColumns + 1);
	setColumnCount(maxColumns);

	int row = 0;
	int col = 0;
	int idx = 0;

	foreach(QColor color, _colors) {
		setItem(row, col, new QTableWidgetItem);
		item(row, col)->setBackground(color);
		col = (col + 1) % maxColumns;
		if (col % maxColumns == 0)
			row++;
	}
}

const ColorList& ColorTableWidget::colors() const
{
	return _colors;
}

ColorPickerWidget::ColorPickerWidget(QWidget *parent)
	: QWidget(parent),
	ui(new Ui::ColorPickerWidget())
{
	ui->setupUi(this);

	ColorList cl;
	
	for (int i = 0; i < 100; i++) {
		QColor color(rand() % 0xffffff);
		try {
			cl.push_back(QColorDialog::standardColor(i));
		} catch(...) {}
	}
	ui->colors->addColors(cl);
	connect(ui->addcolor, &QPushButton::clicked, this, [&]() {
		QColorDialog *d = new QColorDialog(this);
		if (d->exec() == QColorDialog::Accepted) {
			ui->colors->addColors({ d->currentColor() });
		}
	});

	connect(ui->colors, &ColorTableWidget::foregroundColorChanged, this, [&](const QColor& color) {
		QPalette palette = ui->sampleText->palette();
		palette.setColor(ui->sampleText->foregroundRole(), color);
		ui->sampleText->setPalette(palette);
		emit colorChanged(palette.foreground().color(), palette.background().color());
	});

	connect(ui->colors, &ColorTableWidget::backgroundColorChanged, this, [&](const QColor& color) {
		QPalette palette = ui->sampleText->palette();
		palette.setColor(ui->sampleText->backgroundRole(), color);
		ui->sampleText->setPalette(palette);
		emit colorChanged(palette.foreground().color(), palette.background().color());
	});
}

void ColorPickerWidget::setActiveColor(const QColor& fg, const QColor& bg)
{
	QPalette palette = ui->sampleText->palette();
	palette.setColor(ui->sampleText->foregroundRole(), fg);
	palette.setColor(ui->sampleText->backgroundRole(), bg);
	ui->sampleText->setPalette(palette);
}


