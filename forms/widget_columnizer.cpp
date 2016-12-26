#include "columnizerwidget.h"
#include "signalmapper.h"
#include <qtablewidget.h>
#include "settings.h"
#include "Utils/utils.h"
#include <QSqlDatabase>
#include <qcheckbox.h>
#include <qdatetime.h>
#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>

ColumnizerWidget::ColumnizerWidget(QDialog *parent) :
    QDialog(parent),
	ui(new Ui::ColumnizerWidget)
{
	ui->setupUi(this);
	ui->btnAddName->setEnabled(false);
	ui->btnDeletePattern->setEnabled(false);
	
	QStringList headers;
	headers << "Name" << "Pattern" << "Show" << "Fmt" << "From" << "To";
	ui->tableWidget->setColumnCount(headers.size());
	ui->tableWidget->setHorizontalHeaderLabels(headers);
	//ui->tableWidget->verticalHeader()->setVisible(false);
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
	//ui->tablePreview->verticalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);	
	//ui->tablePreview->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Custom);
	//ui->tablePreview->setWordWrap(false);
	//connect(ui->editSubject, SIGNAL(textChanged(QString)), this, SLOT(subjectChanged(QString)));
	loadSettings();
}

void ColumnizerWidget::saveSettings()
{
    Settings settings;
    QString name = ui->cbName->currentText();
	int col = 0;
    if(name.length() > 0) {
		settings.columnizer(name).subject(ui->editSubject->toPlainText());
		for (int row = 0; row < ui->tableWidget->rowCount(); row++)
		{			
			auto r = getPatternRow(row);
			settings.columnizer(name).columns(col).name(r.fieldName);
			settings.columnizer(name).columns(col).pattern(r.pattern);
			settings.columnizer(name).columns(col).enabled(r.enabled);
			settings.columnizer(name).columns(col).fmtFunc(r.fmtFunc);
			settings.columnizer(name).columns(col).fmtFrom(r.fmtFrom);
			settings.columnizer(name).columns(col).fmtTo(r.fmtTo);
			col++;
		}
		settings.columnizer(name).tableState(ui->tablePreview->horizontalHeader()->saveState());
    }
}

void ColumnizerWidget::loadSettings(QString name)
{
    Settings settings;
	removePatternRows();
	ui->cbName->blockSignals(true);
	if (name.length() == 0) {
		auto index = ui->cbName->currentIndex();
		ui->cbName->clear();
		ui->cbName->addItems(settings.childGroups("columnizer"));
		if (index != -1)
			ui->cbName->setCurrentIndex(index);
	}
	if (name.length() == 0)
		name = ui->cbName->currentText();
	ui->cbName->setCurrentIndex(ui->cbName->findText(name));
	ui->cbName->blockSignals(false);
	ui->editSubject->setPlainText(settings.columnizer(name).subject());
	foreach(const QString& col, settings.childGroups("columnizer/" + name + "/columns"))
	{
		PatternRow r;
		r.fieldName = settings.columnizer(name).columns(col).name();
		r.pattern = settings.columnizer(name).columns(col).pattern();
		r.enabled = settings.columnizer(name).columns(col).enabled();
		r.fmtFunc = settings.columnizer(name).columns(col).fmtFunc();
		r.fmtFrom = settings.columnizer(name).columns(col).fmtFrom();
		r.fmtTo = settings.columnizer(name).columns(col).fmtTo();
		addPatternRow(r);
	}
	
	ui->tablePreview->horizontalHeader()->restoreState(settings.columnizer(name).tableState());	
	updatePreview();
}

void ColumnizerWidget::addPatternRow(const PatternRow& r)
{
	QString css = "QWidget { border:0px solid black; }";
	ui->tableWidget->blockSignals(true);
	ui->btnDeletePattern->setEnabled(true);
	ui->tableWidget->insertRow(ui->tableWidget->rowCount());
	auto row = ui->tableWidget->rowCount() - 1;

	QLineEdit *editFieldName = new QLineEdit(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 0, editFieldName);
	editFieldName->setObjectName(QStringLiteral("editFieldName"));
	editFieldName->setText(r.fieldName);
	editFieldName->setStyleSheet(css);
	

	QLineEdit *editPattern = new QLineEdit(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 1, editPattern);
	editPattern->setObjectName(QStringLiteral("editPattern"));
	editPattern->setText(r.pattern);
	editPattern->setStyleSheet(css);
	
	QCheckBox *cbEnabled = new QCheckBox();
	ui->tableWidget->setCellWidget(row, 2, cbEnabled);
	cbEnabled->setObjectName(QStringLiteral("cbEnabled"));
	cbEnabled->setChecked(r.enabled);
	cbEnabled->setStyleSheet(css);

	QComboBox *cbFmtFunc = new QComboBox();
	ui->tableWidget->setCellWidget(row, 3, cbFmtFunc);
	cbFmtFunc->setObjectName(QStringLiteral("editFmtFunc"));
	cbFmtFunc->addItem("None");
	cbFmtFunc->addItem("DateTime");
	cbFmtFunc->addItem("Regex");
	cbFmtFunc->setCurrentText(r.fmtFunc);
	cbFmtFunc->setStyleSheet(css);

	QLineEdit *editFmtFrom = new QLineEdit();
	ui->tableWidget->setCellWidget(row, 4, editFmtFrom);
	editFmtFrom->setObjectName(QStringLiteral("editFmtFrom"));
	editFmtFrom->setText(r.fmtFrom);
	editFmtFrom->setStyleSheet(css);

	QLineEdit *editFmtTo = new QLineEdit();
	ui->tableWidget->setCellWidget(row, 5, editFmtTo);
	editFmtTo->setObjectName(QStringLiteral("editFmtTo"));
	editFmtTo->setText(r.fmtTo);
	editFmtTo->setStyleSheet(css);

	ui->tableWidget->blockSignals(false);
	connect(editFieldName, SIGNAL(textChanged(QString)), this, SLOT(on_editFieldName_textChanged(QString)));
	connect(editPattern, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(editFmtFrom, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(editFmtTo, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(cbEnabled, SIGNAL(toggled(bool)), this, SLOT(on_cbEnabled_toggle(bool)));
	SMapBool_QString sm;
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
	ui->btnDeletePattern->setEnabled(ui->tableWidget->rowCount() > 0);
}

void ColumnizerWidget::removePatternRows()
{
	while (ui->tableWidget->rowCount()) {
		removePatternRow(0);
	}
}

void ColumnizerWidget::updatePreview()
{
	//qDebug() << sender() << pattern;
	auto savedState = ui->tablePreview->horizontalHeader()->saveState();
	while (ui->tablePreview->rowCount())
		ui->tablePreview->removeRow(0);
	//for (int col = 0; col < ui->tablePreview->columnCount(); col++)
	//	ui->tablePreview->setItem(0, col, new QTableWidgetItem(""));
	QString pattern;
	ui->tablePreview->setColumnCount(ui->tableWidget->rowCount());
	
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
    QString currentText = ui->cbName->currentText();
    saveSettings();
    loadSettings(currentText);
}

void ColumnizerWidget::on_btnCancel_clicked()
{
	window()->close();
}

void ColumnizerWidget::on_cbName_currentTextChanged(const QString &arg1)
{
	qDebug() << ui->cbName->findText(arg1);
	ui->btnAddName->setEnabled(ui->cbName->findText(arg1) == -1);
}

void ColumnizerWidget::on_cbName_currentIndexChanged(int index)
{
	QString name = ui->cbName->itemText(index);
	qDebug() << ui->cbName->findText(name);
	ui->btnAddName->setEnabled(ui->cbName->findText(name) == -1);
	if (name.length() > 0)
		loadSettings(name);
}

void ColumnizerWidget::on_btnDelete_clicked()
{
    Settings settings;
    QString name = ui->cbName->currentText();
	settings.columnizer(name).remove();
    loadSettings();
}

void ColumnizerWidget::on_btnAddName_clicked()
{
	Settings settings;
	QString name = ui->cbName->currentText();
	ui->cbName->addItem(name);
	ui->btnAddName->setEnabled(false);
	removePatternRows();

}

void ColumnizerWidget::on_btnAddPattern_clicked()
{
	addPatternRow();
	updatePreview();
}

void ColumnizerWidget::on_btnDeletePattern_clicked()
{
	removePatternRow(ui->tableWidget->currentRow());
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