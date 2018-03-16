#pragma once
#include <TextColorizer.h>

#include <qstandarditemmodel.h>
#include <qlistview.h>
#include <qtoolbutton.h>
#include <qwidgetaction.h>
#include <qcolordialog.h>
#include <qgroupbox.h>
#include <qtableview.h>
#include <qgridlayout.h>

namespace Ui {
	class TextColorizerWidget;
}

class TextColorizerWidget : public QWidget
{
	Q_OBJECT
	Ui::TextColorizerWidget *ui;
	//QTableView _tv;
	//QStandardItemModel _model;

	QToolButton& addToolButton(int row, int col, const QString& name, bool& bind);
	QToolButton& addToolButton(int row, int col, const QString& name, std::function<void(bool checked)> onClick);

	void addRow(boost::optional<TextColorize> colorize = boost::none);
	void removeSelectedRow();
	void enableRowBasedButtons(bool enable);
	TextColorize::Ptr getTextColorize(int row);
	void handleRowChanged(int row);
	void handleItemChanged();
public:
	TextColorizerWidget(QWidget *parent);

	void initialize(const TextColorize::List& colorizeList);

	const TextColorize::List colorizeList() const;

signals:
	void itemsChanged(const TextColorize::List& items);
	void itemChanged(const TextColorize& item);
};

