#include "ColumnizerChooser.h"
#include "signalmapper.h"
#include <qtablewidget.h>
#include "settings.h"
#include "Utils/utils.h"
#include <QSqlDatabase>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

ColumnizerChooser::ColumnizerChooser(QDialog *parent) :
    QDialog(parent),
	ui(new Ui::ColumnizerChooser)
{
	ui->setupUi(this);

}

void ColumnizerChooser::addList(const QStringList & list)
{
	ui->columnizer->addItems(list);
}

QString ColumnizerChooser::columnizer() const
{
	return ui->columnizer->currentItem()->text();
}
