#pragma once
#include <QWidget>
#include <QtUiPlugin/QDesignerExportWidget>
namespace Ui {
class QueryFilter;
}

class QDESIGNER_WIDGET_EXPORT QueryFilter : public QWidget
{
    Q_OBJECT
public:
    explicit QueryFilter(QWidget *parent = 0);
    ~QueryFilter();
private:
    Ui::QueryFilter *ui;
};
