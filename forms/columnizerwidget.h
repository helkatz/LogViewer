#pragma once

#include <QDialog>
#include <QSettings>
#include <qtablewidget.h>
#include <qlineedit.h>
#include "ui_columnizerwidget.h"
class QHBoxLayout;
namespace Ui {
class ColumnizerWidget;
}

class ColumnizerWidget : public QDialog
{
    Q_OBJECT
	class PatternRow
	{
	public:
		QString fieldName;
		QString pattern;
		bool enabled;
		QString fmtFunc;
		QString fmtFrom;
		QString fmtTo;
		QByteArray previewState;
		PatternRow() :
			enabled(true)
		{
		}
	};

	//void addPatternRow(const QString& fieldName = "", const QString& pattern = "", bool enabled = true);
	void addPatternRow(const PatternRow& row = PatternRow());
	PatternRow getPatternRow(int row);
	void removePatternRow(int row);
	void removePatternRows();

	
public:
	explicit ColumnizerWidget(QDialog *parent = 0);
	~ColumnizerWidget();

private slots:

	void updatePreview();

    void on_btnSave_clicked();

    void on_btnCancel_clicked();

    void on_cbName_currentTextChanged(const QString &arg1);

    void on_cbName_currentIndexChanged(int index);

    void on_btnDelete_clicked();

	void on_btnAddPattern_clicked();
	void on_btnDeletePattern_clicked();
	void on_patternRow_activate()
	{
		loadSettings();
	}

	void patternChanged(const QString& pattern);
	void on_editSubject_textChanged();
	void on_editFieldName_textChanged(const QString&);
	void on_editPattern_textChanged(const QString&);

	void on_cbEnabled_toggle(bool);
private:
	Ui::ColumnizerWidget *ui;
    void saveSettings();
    void loadSettings(QString name = "");
	QHBoxLayout *addTableRow();
};


