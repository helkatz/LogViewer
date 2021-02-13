#include "../Columnizer.h"
#include "../Settings.h"
#include <gui/columnizerwidget.h>
#include <ui/ui_columnizerwidget.h>
#include <core/settings.h>
#include <utils/utils.h>

#include <qtablewidget.h>
#include <QSqlDatabase>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

ColumnizerWidget::ColumnizerWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ColumnizerWidget)
{
	ui->setupUi(this);
	ui->deletePatternButton->setEnabled(false);
	ui->cbName->lineEdit()->setPlaceholderText("enter a name and press save");

	QStringList headers;
	headers << "Name" << "Pattern" << "Show" << "Fmt" << "From" << "To";
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	//ui->tableWidget->setSelectionMode(QAbstractItemView::EditingState);
	ui->tableWidget->setColumnCount(headers.size());
	ui->tableWidget->setHorizontalHeaderLabels(headers);
	ui->tableWidget->setTabKeyNavigation(false);
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

	connect(parent, SIGNAL(saveSettings()), this, SLOT(saveSettings()));

	loadSettings();
}

void ColumnizerWidget::saveSettings()
{
	if (ui->cbName->signalsBlocked())
		return;
    QString name = ui->cbName->currentText();
	int col = 0;
    if(name.length() > 0) {
		auto s = settings.columnizers(name);
		s.subject(ui->editSubject->toPlainText());
		s.columns().remove();
		for (int row = 0; row < ui->tableWidget->rowCount(); row++)
		{			
			auto r = getPatternRow(row);
			s.columns(col).name(r.fieldName);
			s.columns(col).pattern(r.pattern);
			s.columns(col).enabled(r.enabled);
			s.columns(col).fmtFunc(r.fmtFunc);
			s.columns(col).fmtFrom(r.fmtFrom);
			s.columns(col).fmtTo(r.fmtTo);
			col++;
		}
		s.tableState(ui->tablePreview->horizontalHeader()->saveState());
    }
}

void ColumnizerWidget::loadSettings()
{	
	auto settings = appSettings().as<LogFileSettings>();
	ui->cbName->blockSignals(true);
	removePatternRows();
	auto selectIndex = ui->cbName->currentIndex();
	ui->cbName->clear();
	ui->cbName->addItems(settings.columnizers().childGroups());

	if (ui->cbName->count() == 0)
		return;
	if (selectIndex == -1) {
		selectIndex = 0;
	}

	ui->cbName->setCurrentIndex(selectIndex);
	auto selectedName = ui->cbName->currentText();
	
	if (selectedName.length()) {
		auto s = settings.columnizers(selectedName);
		ui->editSubject->setPlainText(s.subject());
		foreach(const QString & col, s.columns().childGroups()) {
			PatternRow r;
			r.fieldName = s.columns(col).name();
			r.pattern = s.columns(col).pattern();
			r.enabled = s.columns(col).enabled();
			r.fmtFunc = s.columns(col).fmtFunc();
			r.fmtFrom = s.columns(col).fmtFrom();
			r.fmtTo = s.columns(col).fmtTo();
			addPatternRow(ui->tableWidget->rowCount(), r);
		}
		ui->tablePreview->horizontalHeader()->restoreState(s.tableState());
	}
	
	updatePreview();
	ui->cbName->blockSignals(false);
}

//#define WIDGET QLineEdit
template<typename WIDGET = QWidget>
class TableCellWidgetHook : public WIDGET {
	using WIDGET::WIDGET;
	QTableWidget* tableWidget_;
public:
	TableCellWidgetHook(QTableWidget *parent):
		WIDGET(parent),
		tableWidget_(parent)
	{}

	void focusInEvent(QFocusEvent* event) override
	{		
		tableWidget_->blockSignals(true);
		auto index = tableWidget_->currentIndex();
		for (int row = 0; row < tableWidget_->rowCount(); row++) {
			for (int col = 0; col < tableWidget_->columnCount(); col++) {
				if (tableWidget_->cellWidget(row, col) == this) {
					tableWidget_->selectRow(row);
					tableWidget_->selectColumn(col);
				}
			}
		}
		tableWidget_->blockSignals(false);
		setFocus();
		WIDGET::focusInEvent(event);
	}
};

void ColumnizerWidget::addPatternRow(int row, const PatternRow& r)
{
	QString css = "QWidget { border:0px solid black}";
	ui->tableWidget->blockSignals(true);
	ui->deletePatternButton->setEnabled(true);
	ui->tableWidget->insertRow(row);
	
	//auto row = ui->tableWidget->rowCount() - 1;

	auto *editFieldName = new TableCellWidgetHook<QLineEdit>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 0, editFieldName);
	ui->tableWidget->setColumnWidth(0, 50);
	editFieldName->setObjectName(QStringLiteral("editFieldName"));
	editFieldName->setText(r.fieldName);
	editFieldName->setStyleSheet(css);
	

	QLineEdit *editPattern = new TableCellWidgetHook<QLineEdit>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 1, editPattern);
	editPattern->setObjectName(QStringLiteral("editPattern"));
	editPattern->setText(r.pattern);
	editPattern->setStyleSheet(css);
	
	QCheckBox *cbEnabled = new TableCellWidgetHook<QCheckBox>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 2, cbEnabled);
	cbEnabled->setObjectName(QStringLiteral("cbEnabled"));
	cbEnabled->setChecked(r.enabled);
	cbEnabled->setStyleSheet(css);
	ui->tableWidget->setColumnWidth(2, 20);

	QComboBox *cbFmtFunc = new TableCellWidgetHook<QComboBox>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 3, cbFmtFunc);
	cbFmtFunc->setObjectName(QStringLiteral("editFmtFunc"));
	cbFmtFunc->addItem("None");
	cbFmtFunc->addItem("DateTime");
	cbFmtFunc->addItem("strptime");
	cbFmtFunc->addItem("Regex");
	cbFmtFunc->setCurrentText(r.fmtFunc);
	cbFmtFunc->setStyleSheet(css);

	QLineEdit *editFmtFrom = new TableCellWidgetHook<QLineEdit>(ui->tableWidget);
	//ui->tableWidget->setTabOrder()
	ui->tableWidget->setCellWidget(row, 4, editFmtFrom);
	editFmtFrom->setObjectName(QStringLiteral("editFmtFrom"));
	editFmtFrom->setText(r.fmtFrom);
	editFmtFrom->setStyleSheet(css);

	QLineEdit *editFmtTo = new TableCellWidgetHook<QLineEdit>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 5, editFmtTo);
	editFmtTo->setObjectName(QStringLiteral("editFmtTo"));
	editFmtTo->setText(r.fmtTo);
	editFmtTo->setStyleSheet(css);

	ui->tableWidget->blockSignals(false);
	connect(editFieldName, SIGNAL(textChanged(QString)), this, SLOT(on_editFieldName_textChanged(QString)));
	connect(editPattern, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(cbFmtFunc, SIGNAL(currentIndexChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(editFmtFrom, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(editFmtTo, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(cbEnabled, SIGNAL(toggled(bool)), this, SLOT(on_cbEnabled_toggle(bool)));


	connect(ui->tableWidget, &QTableWidget::cellClicked, this, [](int row, int col)->void {
		qDebug() << "Clicked: " << row << "," << col;
		});

	//SMapBool_QString sm;
	//sm.docon(cbEnabled, SIGNAL(toggled(bool)), this, SLOT(patternChanged(QString)));
}
void ColumnizerWidget::on_cbEnabled_toggle(bool)
{
	patternChanged("");
}

ColumnizerWidget::PatternRow ColumnizerWidget::getPatternRow(int row)
{
	PatternRow r;
	if (ui->tableWidget->cellWidget(row, ui->tableWidget->columnCount() - 1) == nullptr)
		return PatternRow();
	r.fieldName = qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(row, 0))->text();
	r.pattern = qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(row, 1))->text();
	r.enabled = qobject_cast<QCheckBox *>(ui->tableWidget->cellWidget(row, 2))->isChecked();
	r.fmtFunc = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(row, 3))->currentText();
	r.fmtFrom = qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(row, 4))->text();
	r.fmtTo = qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(row, 5))->text();
	return r;
}
void ColumnizerWidget::removePatternRow(int row)
{
	ui->tableWidget->removeRow(row);
	ui->tablePreview->removeColumn(row);
	ui->deletePatternButton->setEnabled(ui->tableWidget->rowCount() > 0);
	saveSettings();
}

void ColumnizerWidget::removePatternRows()
{
	while (ui->tableWidget->rowCount()) {
		removePatternRow(0);
	}
}

void ColumnizerWidget::updatePreview()
{
	saveSettings();
	//qDebug() << sender() << pattern;
	auto savedState = ui->tablePreview->horizontalHeader()->saveState();

	while (ui->tablePreview->rowCount())
		ui->tablePreview->removeRow(0);

	ui->tablePreview->setColumnCount(ui->tableWidget->rowCount());

	QString pattern;
	for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
		PatternRow r = getPatternRow(row);
		pattern += r.pattern;
		/*ui->tablePreview->horizontalHeader()->setSectionResizeMode(row, QHeaderView::Stretch);
		if (row > 0)
			ui->tablePreview->horizontalHeader()->setSectionResizeMode(row - 1, QHeaderView::Custom);*/
		ui->tablePreview->setHorizontalHeaderItem(row, new QTableWidgetItem(r.fieldName));
		//ui->tablePreview->horizontalHeader()->setStretchLastSection(true);
	}
	ui->tablePreview->horizontalHeader()->restoreState(savedState);
	ui->tablePreview->horizontalHeader()->setSectionsMovable(true);
	ui->tablePreview->horizontalHeader()->setStretchLastSection(true);

	if (pattern.length() == 0)
		return;

	QRegularExpression re(pattern, QRegularExpression::DotMatchesEverythingOption
		| QRegularExpression::MultilineOption
		//|QRegularExpression::InvertedGreedinessOption
	);

	QString sampleLines = ui->editSubject->toPlainText();
	QRegularExpression reLines("(.*[^\\n])");
	auto mLines = reLines.globalMatch(ui->editSubject->toPlainText());
	QList<QStringList> sampleEntries;
	while (mLines.hasNext()) {
		auto line = mLines.next().captured(0);
		auto m = re.match(line);
		ui->tablePreview->insertRow(ui->tablePreview->rowCount());
		qDebug() << m.hasMatch() << m.capturedTexts();
		if (m.hasMatch()) {
			auto capturedTexts = m.capturedTexts();
			if(capturedTexts.size() > 1)
				capturedTexts.removeFirst();
			sampleEntries.append(capturedTexts);
		}
		else {
			if (sampleEntries.size())
				sampleEntries.back().back() += "\n" + line;
		}
	}
	ui->tablePreview->setRowCount(sampleEntries.size());
	int previewRow = 0;
	ui->tablePreview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	foreach(const QStringList& sampleEntry, sampleEntries)
	{
		ui->tablePreview->verticalHeader()->setSectionResizeMode(previewRow, QHeaderView::ResizeToContents);
		for (int patternRow = 0; patternRow < ui->tableWidget->rowCount(); patternRow++) {
			PatternRow r = getPatternRow(patternRow);
			ui->tablePreview->setColumnHidden(patternRow, r.enabled == false);

			QString field;
			if(sampleEntry.size() > patternRow)
				field = sampleEntry.at(patternRow);// m.captured(row + 1);
			if (r.fmtFunc.length()) {
				if (r.fmtFunc == "DateTime") {
					QDateTime dt;
					dt.fromString(field.toStdString().c_str(), r.fmtFrom.toStdString().c_str());
					field = dt.toString(r.fmtTo.toStdString().c_str());
				}
				if (r.fmtFunc == "strptime") {
					DateTime dt;
					dt.parseTime(field.toStdString().c_str(), r.fmtFrom.toStdString().c_str());
					field = dt.toString(r.fmtTo.toStdString().c_str()).c_str();
				}
				else if (r.fmtFunc == "Regex") {
					QRegularExpression re(r.fmtFrom, QRegularExpression::DotMatchesEverythingOption 
						| QRegularExpression::MultilineOption
						//| QRegularExpression::InvertedGreedinessOption
					);
					field = field.replace(re, r.fmtTo);
				}
			}
			ui->tablePreview->setItem(previewRow, patternRow, new QTableWidgetItem(field));
			ui->tablePreview->item(previewRow, patternRow)->setTextAlignment(Qt::AlignTop);
		}
		previewRow++;
	}
	ui->tablePreview->horizontalHeader()->setStretchLastSection(true);
}

ColumnizerWidget::~ColumnizerWidget()
{
    delete ui;
}

void ColumnizerWidget::on_btnSave_clicked()
{
    //QString currentText = ui->cbName->currentText();
    //saveSettings();
    //loadSettings(currentText);
}

void ColumnizerWidget::on_btnCancel_clicked()
{
	window()->close();
}

void ColumnizerWidget::on_cbName_currentTextChanged(const QString &arg1)
{
	qDebug() << ui->cbName->findText(arg1);
}

void ColumnizerWidget::on_cbName_currentIndexChanged(int index)
{
	loadSettings();
}

void ColumnizerWidget::on_btnDelete_clicked()
{
    QString name = ui->cbName->currentText();
	//auto index = ui->cbName->currentIndex();
	//ui->cbName->removeItem(index);
	if (name.length() == 0)
		return;

	settings.columnizers(name).remove();
	//name = ui->cbName->currentText();
    loadSettings();
}

void ColumnizerWidget::on_addPatternButton_clicked()
{
	addPatternRow(ui->tableWidget->currentRow());
	updatePreview();
}

void ColumnizerWidget::on_deletePatternButton_clicked()
{
	removePatternRow(ui->tableWidget->currentRow());
}

void ColumnizerWidget::on_patternRow_activate()
{
	loadSettings();
}

void ColumnizerWidget::patternChanged(const QString&)
{
	updatePreview();
}

void ColumnizerWidget::on_editSubject_textChanged()
{
	updatePreview();
}

void ColumnizerWidget::on_editFieldName_textChanged(const QString&)
{
	updatePreview();
}

void ColumnizerWidget::on_editPattern_textChanged(const QString&)
{
	updatePreview();
}