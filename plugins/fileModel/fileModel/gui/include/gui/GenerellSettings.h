#pragma once
#include "../Settings.h"

#include <QDialog>

namespace Ui {
class GenerellWidget;
}

class GenerellWidget : public QWidget//	, public SettingsIO
{
    Q_OBJECT;

    LogFileSettings settings;
public:
    explicit GenerellWidget(QWidget *parent = 0);
    ~GenerellWidget();
private slots:

	void saveSettings();

	void loadSettings();
private:
    Ui::GenerellWidget *ui;
};

