#pragma once
#include <core/settings.h>

#include <QDialog>

namespace Ui {
class GenerellWidget;
}

class GenerellWidget : public QWidget//	, public SettingsIO
{
    Q_OBJECT

public:
    explicit GenerellWidget(QWidget *parent = 0);
    ~GenerellWidget();

	bool followMode() const;
	int limit() const;


private slots:

	void saveSettings();

	void loadSettings();

	void on_logLevel_currentIndexChanged(int);

private:
    Ui::GenerellWidget *ui;
};

