#ifndef COLORPICKER_H
#define COLORPICKER_H

#include <QWidget>
#include <qlist.h>
namespace Ui {
	class ColorPicker;
}

class QTableWidget;
typedef QList<QColor> ColorList;
class ColorPicker : public QWidget
{
	Q_OBJECT

public:
	ColorPicker(QWidget *parent);
	~ColorPicker();

	void setAvailableColors(const ColorList& colors);
	void setUsedColors(const ColorList& colors);
	const ColorList& getAvailableColors() const;
	const ColorList& getUsedColors() const;
	void addUsedColor(const QColor& color);

signals:
	void usedColorsChanged();
	void availableColorsChanged();


private:
	Ui::ColorPicker *ui;
	ColorList _availableColors;
	ColorList _usedColors;

	const int maxColumns = 10;

	void addColorsToTable(const ColorList& colors, QTableWidget *tw);
};

#endif // COLORPICKER_H
