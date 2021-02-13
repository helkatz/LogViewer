#include "NameChooser.h"
#include <ui_NameChooser.h>

#include <QPushButton>
#include <QComboBox>

NameChooser::NameChooser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NameChooser)
{
    ui->setupUi(this);

	connect(ui->remove, &QPushButton::clicked, this, [this]() {
		if (ui->comboName->currentIndex() == -1)
			return;
		ui->comboName->removeItem(ui->comboName->currentIndex());
	});

	connect(ui->comboName, &QComboBox::editTextChanged, this, []() {
	});

}

NameChooser::~NameChooser()
{
}