#ifndef LOGVIEWCONTEXTMENU_H
#define LOGVIEWCONTEXTMENU_H

#include <QMainWindow>

namespace Ui {
class LogViewContextmenu;
}

class LogViewContextmenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit LogViewContextmenu(QWidget *parent = 0);
    ~LogViewContextmenu();

private:
    Ui::LogViewContextmenu *ui;
};

#endif // LOGVIEWCONTEXTMENU_H
