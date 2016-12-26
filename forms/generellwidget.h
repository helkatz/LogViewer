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

	bool followMode() const;
	int limit() const;
	private slots:

	void on_btnSave_clicked();

	void on_btnCancel_clicked();

	void on_logLevel_currentIndexChanged(int);

private:
    Ui::GenerellWidget *ui;
};

#endif // GENERELLWIDGET_H
