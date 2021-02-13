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
	void addPatternRow(int row, const PatternRow& pr = PatternRow());
	PatternRow getPatternRow(int row);
	void removePatternRow(int row);
	void removePatternRows();

	
public:
	explicit ColumnizerWidget(QWidget *parent = 0);
	~ColumnizerWidget();

private slots:
	/// when onlyInCache=true then changes where not stored
	/// but keeps in settings cache
	void saveSettings();
	
	void loadSettings();

	void updatePreview();

    void on_btnSave_clicked();

    void on_btnCancel_clicked();

    void on_cbName_currentTextChanged(const QString &arg1);

    void on_cbName_currentIndexChanged(int index);

    void on_btnDelete_clicked();

	void on_addPatternButton_clicked();
	void on_deletePatternButton_clicked();
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


