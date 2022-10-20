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
#include <boost/regex.hpp>
class RecursivGuard;
class RecursivHold {
	int recursive_;
	friend RecursivGuard;
public:
	RecursivHold() : recursive_(0) {}
};
class RecursivGuard {
	RecursivHold& recursiveHold_;
public:
	RecursivGuard(RecursivHold& recursivHold) :
		recursiveHold_(recursivHold)
	{
		recursiveHold_.recursive_++;
	}
	~RecursivGuard() {
		recursiveHold_.recursive_--;
	}

	void onTeardown(std::function<void()> fn)
	{
		fn();
	}

	bool isRecursive() const
	{
		return recursiveHold_.recursive_ > 1;
	}
};

class BlockSignalGuard
{
	static QMap<QObject*, int> objects_;
	std::initializer_list<QObject*> os_;
public:
	BlockSignalGuard(std::initializer_list<QObject*> os)
		: os_(os)
	{
		for (auto& o : os_) {
			objects_[o]++;
			if (objects_[o] == 1)
				o->blockSignals(true);
		}
	}

	~BlockSignalGuard()
	{
		for (auto& o : os_) {
			objects_[o]--;
			if (objects_[o] == 0)
				o->blockSignals(false);
		}
	}
};
QMap<QObject*, int> BlockSignalGuard::objects_;

ColumnizerWidget::ColumnizerWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ColumnizerWidget)
{
	ui->setupUi(this);
	ui->deletePattern->setEnabled(false);
	ui->insertPatternBelow->setEnabled(true);
	//ui->cbName->lineEdit()->setPlaceholderText("enter a name and press save");

	QStringList headers;
	headers << "Name" << "Pattern" << "Show" << "Fmt" << "From" << "To";
	ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	//ui->tableWidget->setSelectionMode(QAbstractItemView::EditingState);
	ui->tableWidget->setColumnCount(headers.size());
	ui->tableWidget->setHorizontalHeaderLabels(headers);
	ui->tableWidget->setTabKeyNavigation(false);
	ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

	connect(parent, SIGNAL(saveSettings()), this, SLOT(saveSettings()));
	appSettings()->bind(settings);
	//settings->unbind();
	ddx(true);
}

void ColumnizerWidget::saveSettings()
{
	appSettings()->set(settings);
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

					//tableWidget_->setCurrentIndex(index);
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

void ColumnizerWidget::addPatternRow(int row, const LogFileSettings::Columnizer::Column& r)
{
	QString css = "QWidget { border:0px solid black}";
	ui->tableWidget->blockSignals(true);
	ui->deletePattern->setEnabled(true);

	if (row == -1) {
		row = ui->tableWidget->rowCount();
	}

	ui->tableWidget->insertRow(row);

	auto *editFieldName = new TableCellWidgetHook<QLineEdit>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 0, editFieldName);
	ui->tableWidget->setColumnWidth(0, 50);
	editFieldName->setObjectName(QStringLiteral("editFieldName"));
	editFieldName->setText(r.name());
	editFieldName->setStyleSheet(css);
	
	QLineEdit *editPattern = new TableCellWidgetHook<QLineEdit>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 1, editPattern);
	editPattern->setObjectName(QStringLiteral("editPattern"));
	editPattern->setText(r.pattern());
	editPattern->setStyleSheet(css);
	
	QCheckBox *cbEnabled = new TableCellWidgetHook<QCheckBox>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 2, cbEnabled);
	cbEnabled->setObjectName(QStringLiteral("cbEnabled"));
	cbEnabled->setChecked(r.enabled());
	cbEnabled->setStyleSheet(css);
	ui->tableWidget->setColumnWidth(2, 20);

	QComboBox *cbFmtFunc = new TableCellWidgetHook<QComboBox>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 3, cbFmtFunc);
	cbFmtFunc->setObjectName(QStringLiteral("editFmtFunc"));
	cbFmtFunc->addItem("None");
	cbFmtFunc->addItem("DateTime");
	cbFmtFunc->addItem("strptime");
	cbFmtFunc->addItem("Regex");
	cbFmtFunc->setCurrentText(r.fmtFunc());
	cbFmtFunc->setStyleSheet(css);

	QLineEdit *editFmtFrom = new TableCellWidgetHook<QLineEdit>(ui->tableWidget);
	//ui->tableWidget->setTabOrder()
	ui->tableWidget->setCellWidget(row, 4, editFmtFrom);
	editFmtFrom->setObjectName(QStringLiteral("editFmtFrom"));
	editFmtFrom->setText(r.fmtFrom());
	editFmtFrom->setStyleSheet(css);

	QLineEdit *editFmtTo = new TableCellWidgetHook<QLineEdit>(ui->tableWidget);
	ui->tableWidget->setCellWidget(row, 5, editFmtTo);
	editFmtTo->setObjectName(QStringLiteral("editFmtTo"));
	editFmtTo->setText(r.fmtTo());
	editFmtTo->setStyleSheet(css);

	ui->tableWidget->blockSignals(false);
	connect(editFieldName, SIGNAL(textChanged(QString)), this, SLOT(on_editFieldName_textChanged(QString)));
	connect(editPattern, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(cbFmtFunc, SIGNAL(currentIndexChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(editFmtFrom, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	connect(editFmtTo, SIGNAL(textChanged(QString)), this, SLOT(patternChanged(QString)));
	//connect(cbEnabled, SIGNAL(toggled(bool)), this, SLOT(on_cbEnabled_toggle(bool)));
	connect(cbEnabled, &QCheckBox::toggled, [this](bool) { 
		ddx(false); 
	});
	connect(ui->tableWidget->horizontalHeader(), &QHeaderView::sectionResized, this, [this]() {
		ddx(false);
	});
	connect(ui->tableWidget, &QTableWidget::currentCellChanged, [this](int currentRow, int currentColumn, int previousRow, int previousColumn)->void {
		if (currentRow != previousRow) {
			//ddx(false);
		}
	});
	connect(ui->tableWidget, &QTableWidget::cellClicked, this, [](int row, int col)->void {
		qDebug() << "Clicked: " << row << "," << col;
		});

	//SMapBool_QString sm;
	//sm.docon(cbEnabled, SIGNAL(toggled(bool)), this, SLOT(patternChanged(QString)));
}
void ColumnizerWidget::on_cbEnabled_toggle(bool)
{
	ddx(false);
}

void ColumnizerWidget::ddx(bool ddxMemberToComponent)
{
	static RecursivHold recursivHold;
	RecursivGuard guard(recursivHold);
	if(guard.isRecursive())
		return;

	BlockSignalGuard blockSignals({ ui->cbName });

	//auto selectedPattern = ui->tableWidget->currentColumn();
	//ui->tableWidget->selectRow(selectedPattern);

	if (ddxMemberToComponent) {
		auto selectIndex = ui->cbName->currentIndex();
		ui->cbName->clear();
		ui->cbName->addItems(settings.columnizers()->childGroups());

		if (ui->cbName->count() == 0)
			return;
		if (selectIndex == -1) {
			selectIndex = 0;
		}
		ui->cbName->setCurrentIndex(selectIndex);
		auto selectedName = ui->cbName->currentText();

		removePatternRows();
		auto s = settings.columnizers(selectedName);
		ui->editSubject->setPlainText(s.subject());
		foreach(const QString & col, s.columns()->childGroups()) {
			addPatternRow(ui->tableWidget->rowCount(), s.columns(col));
		}
		ui->tableWidget->horizontalHeader()->restoreState(s.patternTableState());
		ui->tablePreview->horizontalHeader()->restoreState(s.tableState());
	}
	else {
		
		auto selectedName = ui->cbName->currentText();
		if (selectedName.length() == 0)
			return;
		auto s = settings.columnizers(selectedName);
		s.subject(ui->editSubject->toPlainText());
		s.columns()->remove();

		int col = 0;
		for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
			s.columns(col)->set(getPatternRow(row));
			col++;
		}
		s.patternTableState(ui->tableWidget->horizontalHeader()->saveState());
		s.tableState(ui->tablePreview->horizontalHeader()->saveState());
	}
	updatePreview();
}

LogFileSettings::Columnizer::Column ColumnizerWidget::getPatternRow(int row)
{
	LogFileSettings::Columnizer::Column r;
	if (ui->tableWidget->cellWidget(row, ui->tableWidget->columnCount() - 1) == nullptr)
		return LogFileSettings::Columnizer::Column();
	r.name(qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(row, 0))->text());
	r.pattern(qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(row, 1))->text());
	r.enabled(qobject_cast<QCheckBox *>(ui->tableWidget->cellWidget(row, 2))->isChecked());
	r.fmtFunc(qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(row, 3))->currentText());
	r.fmtFrom(qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(row, 4))->text());
	r.fmtTo(qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(row, 5))->text());
	return r;
}

void ColumnizerWidget::removePatternRow(int row)
{
	ui->tableWidget->removeRow(row);
	ui->tablePreview->removeColumn(row);
}

void ColumnizerWidget::removePatternRows()
{
	while (ui->tableWidget->rowCount()) {
		removePatternRow(0);
	}
}

void ColumnizerWidget::updateButtons()
{
	auto selectedPattern = ui->tableWidget->currentColumn();
	ui->deletePattern->setEnabled(selectedPattern >= 0);
	ui->insertPatternAbove->setEnabled(selectedPattern >= 0);
}

void ColumnizerWidget::updatePreview()
{
	auto savedState = ui->tablePreview->horizontalHeader()->saveState();

	while (ui->tablePreview->rowCount())
		ui->tablePreview->removeRow(0);

	ui->tablePreview->setColumnCount(ui->tableWidget->rowCount());

	QString pattern;
	for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
		auto r = getPatternRow(row);
		pattern += r.pattern();
		/*ui->tablePreview->horizontalHeader()->setSectionResizeMode(row, QHeaderView::Stretch);
		if (row > 0)
			ui->tablePreview->horizontalHeader()->setSectionResizeMode(row - 1, QHeaderView::Custom);*/
		ui->tablePreview->setHorizontalHeaderItem(row, new QTableWidgetItem(r.name()));
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
	boost::regex bre(pattern.toStdString());
	boost::cmatch bm;
	auto s = ui->editSubject->toPlainText().toStdString();
	if (boost::regex_match(s.c_str(), bm, bre)) {
		for (int i = 0; i < bm.size(); i++) {
			std::string sm; 
			sm = bm[i].first;
			sm = bm[i].second;
		}
		
	}
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
	foreach(const QStringList& sampleEntry, sampleEntries) {
		ui->tablePreview->verticalHeader()->setSectionResizeMode(previewRow, QHeaderView::ResizeToContents);
		for (int patternRow = 0; patternRow < ui->tableWidget->rowCount(); patternRow++) {
			auto r = getPatternRow(patternRow);
			ui->tablePreview->setColumnHidden(patternRow, r.enabled() == false);

			QString field;
			if(sampleEntry.size() > patternRow)
				field = sampleEntry.at(patternRow);// m.captured(row + 1);
			if (r.fmtFunc().length()) {
				if (r.fmtFunc() == "DateTime") {
					QDateTime dt;
					dt.fromString(field.toStdString().c_str(), r.fmtFrom().toStdString().c_str());
					field = dt.toString(r.fmtTo().toStdString().c_str());
				}
				if (r.fmtFunc() == "strptime") {
					DateTime dt;
					dt.parseTime(field.toStdString().c_str(), r.fmtFrom().toStdString().c_str());
					field = dt.toString(r.fmtTo().toStdString().c_str()).c_str();
				}
				else if (r.fmtFunc() == "Regex") {
					QRegularExpression re(r.fmtFrom(), QRegularExpression::DotMatchesEverythingOption 
						| QRegularExpression::MultilineOption
						//| QRegularExpression::InvertedGreedinessOption
					);
					field = field.replace(re, r.fmtTo());
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

void ColumnizerWidget::on_cbName_currentTextChanged(const QString &newText)
{
	auto currentText = ui->cbName->itemText(ui->cbName->currentIndex());
	if (currentText != newText) {
		//ui->cbName->setItemText(ui->cbName->currentIndex(), newText);
		//ddx(false);
	}	
}

void ColumnizerWidget::on_cbName_currentIndexChanged(int index)
{
	ddx(true);
}

void ColumnizerWidget::on_insertPatternAbove_clicked()
{
	addPatternRow(ui->tableWidget->currentRow());
	ddx(false);
}

void ColumnizerWidget::on_insertPatternBelow_clicked()
{
	addPatternRow(ui->tableWidget->currentRow() + 1);
	ddx(false);
}

void ColumnizerWidget::on_deletePattern_clicked()
{
	removePatternRow(ui->tableWidget->currentRow());
	ddx(false);
}

void ColumnizerWidget::on_renameColumnizer_clicked()
{
	ui->cbName->setEditable(true);
	auto oldName = ui->cbName->lineEdit()->text();
	/*auto edit = new QLineEdit(ui->cbName);
	ui->cbName->setLineEdit(edit);*/
	ui->cbName->lineEdit()->setFocus();
	ui->cbName->blockSignals(true);
	connect(ui->cbName->lineEdit(), &QLineEdit::editingFinished, [this, oldName]() {		
		auto index = ui->cbName->currentIndex();
		auto name = ui->cbName->lineEdit()->text();
		ui->cbName->setItemText(ui->cbName->currentIndex(), ui->cbName->lineEdit()->text());
		ui->cbName->setEditable(false);
		ui->cbName->blockSignals(false);
		settings.columnizers(oldName)->rename(name);
		qDebug() << "editing finished";
		ddx(true);
	});
}

void ColumnizerWidget::on_duplicateColumnizer_clicked()
{
	auto selectedIndex = ui->cbName->currentIndex();
	if (selectedIndex < 0)
		return;
	auto currentColumnizer = ui->cbName->currentText();
	QString newColumnizer;
	int copyIndex = 1;
	do {
		newColumnizer = QString("%1_Copy%2").arg(currentColumnizer).arg(copyIndex++);
	} while (ui->cbName->findText(newColumnizer) >= 0);
	ui->cbName->addItem(newColumnizer);
	auto newIndex = ui->cbName->findText(newColumnizer);

	auto current = settings.columnizers(currentColumnizer);
	settings.columnizers(newColumnizer)->set(current);
	ddx(true);
}

void ColumnizerWidget::on_deleteColumnizer_clicked()
{
	auto currentColumnizer = ui->cbName->currentText();
	if (currentColumnizer.length() == 0)
		return;
	settings.columnizers(currentColumnizer)->remove();
	ddx(true);
}

void ColumnizerWidget::on_patternRow_activate()
{
	ddx(true);
}

void ColumnizerWidget::patternChanged(const QString&)
{
	ddx(false);
}

void ColumnizerWidget::on_editSubject_textChanged()
{
	ddx(false);
}

void ColumnizerWidget::on_editFieldName_textChanged(const QString&)
{
	ddx(false);
}

void ColumnizerWidget::on_editPattern_textChanged(const QString&)
{
	ddx(false);
}