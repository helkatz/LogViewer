#include "TextColorizerWidget.h"
#include "ColorPickerWidget.h"
#include "ui_TextColorizerWidget.h"
#include "commonWidgets.h"
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qheaderview.h>
#include <qlineedit.h>
#include <qdebug.h>
#include <qmenu.h>


QToolButton& TextColorizerWidget::addToolButton(int row, int col, const QString& name, bool& bind)
{
	auto& btn = addToolButton(row, col, name, [&bind](bool checked) { bind = checked; });
	btn.setChecked(bind);
	return btn;
}

QToolButton& TextColorizerWidget::addToolButton(int row, int col, const QString& name, std::function<void(bool checked)> onClick)
{
	auto btn = new QToolButton(ui->items);
	btn->setObjectName(name);
	btn->setCheckable(true);
	//btn->setMaximumWidth(22);
	QTableWidgetItem item;
	ui->items->setCellWidget(row, col, btn);
	connect(btn, &QToolButton::clicked, this, onClick);
	return *btn;
}

TextColorizerWidget::TextColorizerWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TextColorizerWidget)
{
	ui->setupUi(this);
	ui->items->setSelectionMode(QAbstractItemView::NoSelection);

	enableRowBasedButtons(false);

	connect(ui->add, &QPushButton::clicked, this, [this]() {
		addRow();
	});

	connect(ui->remove, &QPushButton::clicked, this, [this]() {
		removeSelectedRow();
	});
	/*
	connect(ui->items, &QTableWidget::itemChanged, this, [this]() { handleItemChanged(); });

	connect(ui->items->model(), &QAbstractItemModel::dataChanged, this, [this]() { 
		qDebug() << "dataChanged";
		handleItemChanged(); 
	});
	*/
	ColorPickerWidget * colorPicker = new ColorPickerWidget(this);
	colorPicker->setWindowFlags(Qt::Popup);
	connect(colorPicker, &ColorPickerWidget::colorChanged, this, [&](const QColor& fg, const QColor& bg) {
		auto row = ui->items->currentRow();
		auto edit = reinterpret_cast<commonwidgets::LineEdit *>(ui->items->cellWidget(row, 0));
		QPalette palette = edit->palette();
		palette.setColor(edit->foregroundRole(), fg);
		palette.setColor(edit->backgroundRole(), bg);
		edit->setPalette(palette);
		handleItemChanged();
	});

	auto action = new QWidgetAction(ui->color);
	action->setDefaultWidget(colorPicker);
	auto menu = new QMenu(ui->color);
	menu->addAction(action);
	ui->color->setMenu(menu);
	connect(menu, &QMenu::aboutToShow, this, [=]() {
		auto row = ui->items->currentRow();
		menu->show();
		auto it = ui->items->item(row, 0);
		auto edit = reinterpret_cast<commonwidgets::LineEdit *>(ui->items->cellWidget(row, 0));
		colorPicker->setActiveColor(
			edit->palette().color(edit->foregroundRole()), 
			edit->palette().color(edit->backgroundRole()));
		colorPicker->show();
		colorPicker->setFocus();
	});

	connect(ui->bold, &QToolButton::clicked, this, [this]() { handleItemChanged(); });
	connect(ui->wholeWord, &QToolButton::clicked, this, [this]() { handleItemChanged(); });
	connect(ui->caseSensitive, &QToolButton::clicked, this, [this]() { handleItemChanged(); });
	connect(ui->wholeRow, &QToolButton::clicked, this, [this]() { handleItemChanged(); });
	connect(ui->wholeCell, &QToolButton::clicked, this, [this]() { handleItemChanged(); });
	connect(ui->regex, &QToolButton::clicked, this, [this]() { handleItemChanged(); });

	connect(ui->items, &QTableWidget::currentCellChanged, this, [this](int row, int col, int, int) {
		handleRowChanged(row);
	});
	return;
}

void TextColorizerWidget::handleItemChanged()
{
	auto row = ui->items->currentRow();
	if (row < 0)
		return;
	//ui->items->blockSignals(true);
	auto item = ui->items->item(row, 0);
	ui->items->item(row, 0)->data(Qt::UserRole).value<TextColorize>();
	auto& colorize = *getTextColorize(row);
	auto edit = reinterpret_cast<commonwidgets::LineEdit *>(ui->items->cellWidget(row, 0));
	
	colorize.text = edit->text();
	colorize.font.setBold(ui->bold->isChecked());
	colorize.useRegex = ui->regex->isChecked();
	colorize.wordOnly = ui->wholeWord->isChecked();
	colorize.caseSensitive = ui->caseSensitive->isChecked();
	colorize.wholeRow = ui->wholeRow->isChecked();
	colorize.wholeCell = ui->wholeCell->isChecked();

	colorize.foregroundColor = edit->palette().color(edit->foregroundRole());// item->foreground().color();
	colorize.backgroundColor = edit->palette().color(edit->backgroundRole());//item->background().color();
	//item->setData(Qt::UserRole, QVariant::fromValue<TextColorize>(colorize));
	//ui->items->blockSignals(false);
	emit itemsChanged(colorizeList());
};

TextColorize::Ptr TextColorizerWidget::getTextColorize(int row)
{
	return ui->items->item(row, 0)->data(Qt::UserRole).value<TextColorize::Ptr>();
}

void TextColorizerWidget::handleRowChanged(int row)
{
	qDebug() << "handleRowChanged " << row;
	enableRowBasedButtons(row >= 0);
	if (row < 0)
		return;
	auto& colorize = *getTextColorize(row);
	ui->regex->setChecked(colorize.useRegex);
	ui->wholeRow->setChecked(colorize.wholeRow);
	ui->wholeCell->setChecked(colorize.wholeCell);
	ui->caseSensitive->setChecked(colorize.caseSensitive);
	ui->wholeWord->setChecked(colorize.wordOnly);
	ui->bold->setChecked(colorize.font.bold());
}

void TextColorizerWidget::addRow(boost::optional<TextColorize> colorize)
{
	ui->items->insertRow(ui->items->rowCount());
	auto row = ui->items->model()->rowCount() - 1;
	if (!colorize) {
		colorize = TextColorize{};
		colorize->text = "enter";
		colorize->foregroundColor = QColor(Qt::GlobalColor::green);
		colorize->backgroundColor = QColor(Qt::GlobalColor::red);
	}

	// here need an LineEdit instead of standard QTableWidgetItem
	// because i need changes on text change
	auto edit = new commonwidgets::LineEdit(ui->items);
	edit->setText(colorize->text);
	QPalette palette = edit->palette();
	palette.setColor(edit->foregroundRole(), colorize->foregroundColor);
	palette.setColor(edit->backgroundRole(), colorize->backgroundColor);
	edit->setPalette(palette);
	ui->items->setCellWidget(row, 0, edit);

	connect(edit, &commonwidgets::LineEdit::focusIn, this, [this, row]() {	
		auto edit = dynamic_cast<commonwidgets::LineEdit *>(QObject::sender());
		// hackish way to find underlaying row when edit gets focus
		for (int row = 0; row < ui->items->rowCount(); ++row) {
			if(ui->items->cellWidget(row, 0) == edit)
				ui->items->selectRow(row);
		}
	});

	connect(edit, &commonwidgets::LineEdit::textChanged, this, [this](const QString& text) {
		handleItemChanged();
	});

	TextColorize::Ptr colorizePtr = TextColorize::Ptr(new TextColorize{ colorize.get() });

	auto item = new QTableWidgetItem;
	item->setData(Qt::UserRole, QVariant::fromValue<TextColorize::Ptr>(colorizePtr));
	ui->items->setItem(row, 0, item);

	ui->items->selectRow(row);
}

void TextColorizerWidget::removeSelectedRow()
{
	auto row = ui->items->currentRow();
	ui->items->removeRow(row);
	emit itemsChanged(colorizeList());
}

void TextColorizerWidget::enableRowBasedButtons(bool enable)
{
	ui->regex->setEnabled(enable);
	ui->caseSensitive->setEnabled(enable);
	ui->wholeWord->setEnabled(enable);
	ui->wholeRow->setEnabled(enable);
	ui->wholeCell->setEnabled(enable);
	ui->color->setEnabled(enable);
	ui->bold->setEnabled(enable);
	ui->remove->setEnabled(enable);
}

void TextColorizerWidget::initialize(const TextColorize::List& colorizeList)
{
	ui->items->setRowCount(0);
	ui->items->setColumnCount(1);

	for(auto colorize: colorizeList) {	
		addRow(colorize);
	}

	ui->items->resizeRowsToContents();
	ui->items->resizeColumnsToContents();

	ui->items->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

const TextColorize::List TextColorizerWidget::colorizeList() const
{
	TextColorize::List ret;
	for (auto row = 0; row < ui->items->rowCount(); ++row)
		ret.push_back(*const_cast<TextColorizerWidget*>(this)->getTextColorize(row));
	return ret;
}
