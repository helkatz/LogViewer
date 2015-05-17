#ifndef GENERELLWIDGET_H
#define GENERELLWIDGET_H

#include <QDialog>

namespace Ui {
class GenerellWidget;
}

class GenerellWidget : public QDialog
{
    Q_OBJECT

public:
    explicit GenerellWidget(QWidget *parent = 0);
    ~GenerellWidget();

private:
    Ui::GenerellWidget *ui;
};

#endif // GENERELLWIDGET_H
