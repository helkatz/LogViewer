#include "forms/contextmenudetailview.h"
#include "ui_contextmenudetailview.h"

ContextMenuDetailView::ContextMenuDetailView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContextMenuDetailView)
{
    ui->setupUi(this);
}

ContextMenuDetailView::~ContextMenuDetailView()
{
    delete ui;
}

QFontComboBox *ContextMenuDetailView::getFontCombo()
{
    return ui->cbFontStyle;
}

QSpinBox *ContextMenuDetailView::getFontSizeSpin()
{
    return ui->spinFontSize;
}
