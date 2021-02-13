#include <gui/logview/LogView.h>
#include <gui/FontStyleWidget.h>
#include <gui/logview/SyntaxHighLighter.h>
#include <gui/logview/MessageFormatter.h>
#include <qcheckbox.h>
#include <qwidgetaction.h>
#include <qscrollbar.h>
#include <qmenu.h>
#include <qthread.h>
#include <boost/regex.hpp>
#include <QRegExp>
#include <QDebug>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QJsonDocument>


DetailView::DetailView(QWidget *parent):
	QTextEdit(parent),
	_highligter(document())
{	
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
}

void DetailView::writeSettings(_settings::LogWindow& s)
{	
	s.detailView().font(this->font());
	QStringList l;
	for (int col = 0; col < _logModel->columnCount(); col++) {
		if(_visibleColumns[col])
			l << QString("%1").arg(col);
	}
	s.detailView().visbible(l);
}

void DetailView::readSettings(_settings::LogWindow& s)
{
	setFont(s.detailView().font());
	for (auto visibleCol : s.detailView().visbible()) {
		_visibleColumns[visibleCol.toInt()] = true;
	}
}

void DetailView::contextMenu(const QPoint& point)
{
	//ContextMenuDetailView menu(this);
	QMenu menu(this);

	QMenu *visbleColumnsMenu = menu.addMenu(tr("Visible columns"));
	QSqlRecord cols = _logModel->columnsInformation();
	for(int col = 0; col < _logModel->columnCount(); col++) {
		QCheckBox *checkBox = new QCheckBox(&menu);
		checkBox->setText(cols.fieldName(col));
		checkBox->setChecked(_visibleColumns[col]);
		QWidgetAction *checkableAction = new QWidgetAction(&menu);
		checkableAction->setDefaultWidget(checkBox);
		visbleColumnsMenu->addAction(checkableAction);
		connect(checkBox, &QCheckBox::toggled, this, [this, col](bool visible) {
			showColumn(visible, col);
			_visibleColumns[col] = visible;
		});
	}

	FontStyleWidget *fontWidget = new FontStyleWidget(&menu);
	fontWidget->getFontCombo()->setFont(font());

	connect(fontWidget->getFontCombo(), SIGNAL(currentFontChanged(QFont)), this, SLOT(setFont(QFont)));
	fontWidget->getFontSizeSpin()->setValue(font().pointSize());
	connect(fontWidget->getFontSizeSpin(), SIGNAL(valueChanged(int)), this, SLOT(setFontSize(int)));


	QWidgetAction *actionFont = new QWidgetAction(&menu);
	actionFont->setDefaultWidget(fontWidget);
	menu.addMenu(tr("Font"))->addAction(actionFont);
/*
	QFontComboBox *fontCombo = new QFontComboBox(&menu);
	actionFont->setDefaultWidget(fontCombo);
	menu.addMenu(tr("Font"))->addAction(actionFont);
	connect(fontCombo, SIGNAL(currentFontChanged(QFont)), this, SLOT(setFont(QFont)));
*/
	menu.exec(mapToGlobal(point));

	qDebug() << "done contextmenu";
}

void DetailView::showColumn(bool visible, int col)
{
	_visibleColumns[col] = visible;
}

void DetailView::setModel(LogModel *model)
{
	_logModel = model;
}

void DetailView::currentRowChanged(QModelIndex current, QModelIndex last)
{
	QString out;
	for (int col = 0; col < _logModel->columnCount(); col++) {
		if (_visibleColumns[col]) {
			out += _logModel->data(current.sibling(current.row(), col), Qt::DisplayRole).toString() + "<br>";
		}
	}
	//MessageFormatter formatter;
	//formatter.format(out);
	////Highlighter highlighter(document());
	//this->setText(out);
	//return;
	_lines = out.split("\n");
	_currentIndex = current;
	auto setText = [this](quint32 fromLine) {
		QFontMetrics metrics = fontMetrics();
		auto itemsPerPage = height() / metrics.height();
		log_trace(0) << "changed" << fromLine << "itemsPerPage" << itemsPerPage;
		QString text;
		for (auto index = fromLine; index < fromLine + itemsPerPage && index < _lines.size(); ++index)
			text += _lines.at(index) + "\n";
		blockSignals(true);		
		verticalScrollBar()->blockSignals(true);
		this->setText(text);
		verticalScrollBar()->setMaximum(_lines.size() - itemsPerPage);
		//verticalScrollBar()->setValue(fromLine);
		verticalScrollBar()->blockSignals(false);

		blockSignals(false);
	};
	auto displayText = [this](QModelIndex index, bool full) {
		QString out;
		for (int col = 0; col < _logModel->columnCount(); col++) {
			if (_visibleColumns[col]) {
				out += _logModel->data(index.sibling(index.row(), col), Qt::DisplayRole).toString() + "<br>";
			}
		}
		QFontMetrics metrics = fontMetrics();
		auto itemsPerPage = height() / metrics.height();
		//log_trace(0) << "changed" << fromLine << "itemsPerPage" << itemsPerPage;
		//_lines = out.split("\n");
		//out = utils::ReplaceAll(out.toStdString(), "\n", "<br>").c_str();
		//out = utils::ReplaceAll(out.toStdString(), " ", "&nbsp;").c_str();

		int length = 0;
		if(full)
			length = out.length();
		else {
			int countLeft = itemsPerPage + 10;
			
			for (int i = 0; i < out.length() && countLeft; ++i) {
				if (out[i] == '\n')
					countLeft--;
				length++;
			}	
		}
		out.resize(length);
		//blockSignals(true);
		//verticalScrollBar()->blockSignals(true);
		this->setText(out);
		//verticalScrollBar()->setMaximum(_lines.size() - itemsPerPage);
		
		//verticalScrollBar()->blockSignals(false);

		//blockSignals(false);
	};

	_conn = connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this, setText, displayText, current](int value) {
		setText(value);
		//QObject::disconnect(_conn);
		//displayText(_currentIndex, true);
		//verticalScrollBar()->setValue(value);
		
	});
	setText(0);
	//displayText(_currentIndex, false);
	//this->is
	//out = utils::ReplaceAll(out.toStdString(), "\n", "<br>").c_str();
	//out = utils::ReplaceAll(out.toStdString(), " ", "&nbsp;").c_str();
	//QVariant v = _logModel->data(current, Qt::DisplayRole);
	//this->setText(out);

	//_doc.setHtml(out);
	//this->setDocument(&_doc);
	//this->setPlainText(out);
	this->setHtml(out);
}


