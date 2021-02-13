#include <gui/ColumnizerChooser.h>
#include <ui/ui_columnizerchooser.h>


ColumnizerChooser::ColumnizerChooser(QDialog *parent) :
    QDialog(parent),
	ui(new Ui::ColumnizerChooser)
{
	ui->setupUi(this);
	ui->label->setText("choose columnizer fro file");

}

void ColumnizerChooser::addList(const QStringList & list)
{
	ui->columnizer->addItems(list);
}

QString ColumnizerChooser::columnizer() const
{
	return ui->columnizer->currentItem()->text();
}
