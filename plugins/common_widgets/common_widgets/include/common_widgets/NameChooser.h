#pragma once
#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>
namespace Ui {
class NameChooser;
}

class QDESIGNER_WIDGET_EXPORT NameChooser : public QWidget
{
    Q_OBJECT
public:
    explicit NameChooser(QWidget *parent = 0);
    ~NameChooser();
private:
    Ui::NameChooser *ui;
};
