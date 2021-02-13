#pragma once
#include <core/common.h>

#include <QWidget>
#include <QDialog>
#include <QFormLayout>
namespace Ui {
class OpenerDialog;
}

class CORE_API OpenerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit OpenerDialog(QWidget *parent = 0);
    ~OpenerDialog();

	QFormLayout *properties();
private:
    Ui::OpenerDialog *ui;

};
