#ifndef CONTEXTMENUDETAILVIEW_H
#define CONTEXTMENUDETAILVIEW_H

#include <QWidget>

namespace Ui {
class ContextMenuDetailView;
}

class ContextMenuDetailView : public QWidget
{
    Q_OBJECT

public:
    explicit ContextMenuDetailView(QWidget *parent = 0);
    ~ContextMenuDetailView();

    QFontComboBox *getFontCombo();
    QSpinBox *getFontSizeSpin();

private:
    Ui::ContextMenuDetailView *ui;
};

#endif // CONTEXTMENUDETAILVIEW_H
