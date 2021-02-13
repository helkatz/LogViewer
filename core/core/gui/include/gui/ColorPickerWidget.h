#pragma once

#include <QWidget>
#include <qlist.h>
#include <qmap.h>
#include <qtablewidget.h>

namespace Ui {
	class ColorPickerWidget;
}

class QTableWidget;

typedef QList<QColor> ColorList;
class ColorTableWidget: public QTableWidget
{
	Q_OBJECT

	ColorList _colors;
	const int maxColumns = 10;

	void resizeEvent(QResizeEvent *event);
public:
	ColorTableWidget(QWidget *parent);
	~ColorTableWidget();

	void addColors(const ColorList& color);
	
	const ColorList& colors() const;

signals:
	void colorsChanged();
	void foregroundColorChanged(const QColor& color);
	void backgroundColorChanged(const QColor& color);
};

class ColorPickerWidget : public QWidget
{
	Q_OBJECT

	Ui::ColorPickerWidget *ui;
public:
	ColorPickerWidget(QWidget *parent);

	void setActiveColor(const QColor& fg, const QColor& bg);
signals:
	void colorChanged(const QColor& fg, const QColor& bg);
};

