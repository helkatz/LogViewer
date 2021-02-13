#include <common_widgets/QueryFilter.h>
#include <ui_QueryFilter.h>

#include <QPushButton>
#include <QComboBox>
QueryFilter::QueryFilter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QueryFilter)
{
    ui->setupUi(this);
/*	
	connect(ui->remove, &QPushButton::clicked, this, [this]() {
		if (ui->comboName->currentIndex() == -1)
			return;
		ui->comboName->removeItem(ui->comboName->currentIndex());
	});

	connect(ui->comboName, &QComboBox::editTextChanged, this, []() {
	});
	*/
}

QueryFilter::~QueryFilter()
{
}