#include <core/settings.h>
#include <gui/OpenerDialog.h>
#include <ui/ui_OpenerDialog.h>

#include <QCheckBox>
#include <QLineEdit>
OpenerDialog::OpenerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OpenerDialog)
{
    ui->setupUi(this);
/*	connect(ui->nameChosser, &QPushButton::clicked, this, [this]() {
	});*/
}

QFormLayout *OpenerDialog::properties()
{
	return ui->properties;
}

OpenerDialog::~OpenerDialog()
{
}