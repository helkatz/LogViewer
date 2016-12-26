#include "findwidget.h"
#include "ui_findwidget.h"
#include <QMenu>
#include <QCheckBox>
#include <QWidgetAction>
#include <QMenuBar>
FindWidget::FindWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FindWidget)
{
    ui->setupUi(this);
    QMenuBar *menuBar = new QMenuBar(this);
    menuBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    ui->horizontalLayoutFind->insertWidget(0, menuBar);
    columnsMenu = menuBar->addMenu(tr("Columns"));
}

FindWidget::~FindWidget()
{
    delete ui;
}

void FindWidget::setFields(QStringList fields)
{

}

void FindWidget::addSearchField(const QString &field, bool preferred)
{
    QCheckBox *checkBox = new QCheckBox(columnsMenu);
    checkBox->setText(field);
    checkBox->setChecked(preferred);
    QWidgetAction *checkableAction = new QWidgetAction(columnsMenu);
    checkableAction->setDefaultWidget(checkBox);
    columnsMenu->addAction(checkableAction);
}

void FindWidget::setFocus()
{
    ui->editWhere->setFocus();
}

QStringList FindWidget::searchCoumns() const
{
    QStringList ret;
    foreach(QAction *action, columnsMenu->actions()) {
        QCheckBox *cb = qobject_cast<QCheckBox *>((qobject_cast<QWidgetAction *>(action))->defaultWidget());
        if(cb->checkState() == Qt::Checked)
            ret.append(cb->text());
    }
    return ret;
}

void FindWidget::on_btnDown_clicked()
{
	emit find(searchCoumns(), ui->editWhere->text(), false, true);
}

void FindWidget::on_btnUp_clicked()
{
	emit find(searchCoumns(), ui->editWhere->text(), false, false);
}

void FindWidget::on_btnClose_clicked()
{
    hide();
}

void FindWidget::on_editWhere_textChanged(const QString &arg1)
{
	emit setFindTextColor(searchCoumns(), arg1);
}

QString FindWidget::buildSql()
{
    QString sql;
    QString search = ui->editWhere->text();
    QStringList columnsList = searchCoumns();
    if(columnsList.empty())
        return search;
    foreach(QString column, columnsList) {
        sql += QString("or `%1` like('%%2%') ").arg(column).arg(search);
    }
    sql = "(" + sql.mid(3) + ")";
    return sql;
}
