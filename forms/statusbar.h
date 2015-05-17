#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <QWidget>

namespace Ui {
class Statusbar;
}

class Statusbar : public QWidget
{
    Q_OBJECT

public:
    explicit Statusbar(QWidget *parent = 0);
    ~Statusbar();

private:
    Ui::Statusbar *ui;
};

#endif // STATUSBAR_H
