#pragma once
#include "../Settings.h"

#include <QDialog>
#include <QSettings>
#include <qtablewidget.h>
#include <qlineedit.h>

class QHBoxLayout;
namespace Ui {
class ColumnizerWidget;
}

class ColumnizerWidget : public QWidget
{
	Q_OBJECT;

	LogFileSettings settings;

	void addPatternRow(int row, const LogFileSettings::Columnizer::Column& pr = {});
	void ddx(bool ddxMemberToComponent);
	LogFileSettings::Columnizer::Column getPatternRow(int row);
	void removePatternRow(int row);
	void removePatternRows();

	void updateButtons();
public:
	explicit ColumnizerWidget(QWidget *parent = 0);
	~ColumnizerWidget();

private slots:
	/// when onlyInCache=true then changes where not stored
	/// but keeps in settings cache
	void saveSettings();

	void updatePreview();

    void on_cbName_currentTextChanged(const QString &arg1);

    void on_cbName_currentIndexChanged(int index);

	void on_insertPatternAbove_clicked();
	void on_insertPatternBelow_clicked();
	void on_deletePattern_clicked();
	void on_renameColumnizer_clicked();
	void on_duplicateColumnizer_clicked();
	void on_deleteColumnizer_clicked();

	void on_patternRow_activate();
	void patternChanged(const QString& pattern);
	void on_editSubject_textChanged();
	void on_editFieldName_textChanged(const QString&);
	void on_editPattern_textChanged(const QString&);

	void on_cbEnabled_toggle(bool);
private:
	Ui::ColumnizerWidget *ui;
	QHBoxLayout *addTableRow();
};


