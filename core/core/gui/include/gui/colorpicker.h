#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <core/types.h>

#include <QWidget>
#include <qlist.h>
#include <qtablewidget.h>

namespace Ui {
	class ColorPicker;
}

class QTableWidget;
class ColorPicker : public QWidget
{
	Q_OBJECT

public:
	ColorPicker(QWidget *parent);
	~ColorPicker();

	void setAvailableColors(const types::ColorList& colors);
	void setUsedColors(const types::ColorList& colors);
	const types::ColorList& getAvailableColors() const;
	const types::ColorList& getUsedColors() const;
	void addUsedColor(const QColor& color);

signals:
	void usedColorsChanged();
	void availableColorsChanged();


private:
	Ui::ColorPicker *ui;
	types::ColorList _availableColors;
	types::ColorList _usedColors;

	const int maxColumns = 10;

	void addColorsToTable(const types::ColorList& colors, QTableWidget *tw);
};

#endif // COLORPICKER_H
