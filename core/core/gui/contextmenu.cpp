#include <gui/contextmenu.h>
#include <ui/ui_contextmenu.h>

ContextMenu::ContextMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContextMenu)
{
    ui->setupUi(this);
}

ContextMenu::~ContextMenu()
{
    delete ui;
}
