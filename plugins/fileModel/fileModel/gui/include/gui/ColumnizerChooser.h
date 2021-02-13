#pragma once
#include <QDialog>
#include <qstringlist.h>

class QHBoxLayout;
namespace Ui {
class ColumnizerChooser;
}

class ColumnizerChooser : public QDialog
{
    Q_OBJECT

public:
	explicit ColumnizerChooser(QDialog *parent = 0);

	void addList(const QStringList& list);

	QString columnizer() const;
private slots:

private:
	Ui::ColumnizerChooser *ui;
};


