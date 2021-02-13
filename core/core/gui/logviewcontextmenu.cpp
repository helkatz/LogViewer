#include <gui/logviewcontextmenu.h>
#include <ui/ui_logviewcontextmenu.h>

LogViewContextmenu::LogViewContextmenu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LogViewContextmenu)
{
    ui->setupUi(this);
}

LogViewContextmenu::~LogViewContextmenu()
{
    delete ui;
}
