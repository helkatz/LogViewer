#include "statusbar.h"
#include "ui_statusbar.h"

class Base
{

};
class Base1:public Base
{
};
class Base2:public Base
{
};

void func(const Base& base)
{

}

void test()
{
    func(Base1());
    func(Base2());
}

Statusbar::Statusbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Statusbar)
{
    ui->setupUi(this);

}

Statusbar::~Statusbar()
{
    delete ui;
}
