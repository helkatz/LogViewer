#include "logview.h"
#include "logsqlmodel.h"
#include "logfilemodel.h"
#include "mainwindow.h"
#include "forms/finddialog.h"
#include "signalmapper.h"
#include "forms/rowlayoutwidget.h"
#include "forms/fontstylewidget.h"
#include "forms/contextmenu.h"
#include "forms/contextmenufilterdialog.h"

#include <QTWidgets>
#include <QDebug>
#include <QSignalMapper>
#include <QKeySequence>
#include <QKeyEvent>
#include <QKeyEventTransition>
#if 1
class LogModel;

void LogView::keyPressEvent(QKeyEvent *e)
{
	static QTime lastTime(QTime::currentTime());
	QTime currentTime(QTime::currentTime());
	static int speed = 0;
	QModelIndex index = currentIndex();
	int delta = 0;
	auto indexTop = indexAt(QPoint(0, 0));
	auto indexBottom = indexAt(QPoint(0, size().height() - 20));
	int itemsPerPage = verticalScrollBar()->pageStep();  //indexBottom.row() - indexTop.row() - 2;

	if (speed == 0 || lastTime.msecsTo(currentTime) > 300)
		speed = 1;
	else
		speed += 1;
	lastTime = currentTime;
	itemsPerPage *= speed;
	switch(e->key()) {
	case Qt::Key_Up:
		delta = -1;
		break;
	case Qt::Key_Down:
		delta = 1;
		break;
	case Qt::Key_PageUp:
		delta = -itemsPerPage;
		break;
	case Qt::Key_PageDown:
		delta = itemsPerPage;
		break;
	case Qt::Key_Home:
		delta = -index.row();
		break;
	case Qt::Key_End:
		delta = model()->rowCount() - index.row() - 1;
		break;
	default:
		return;
	}

	if (delta < 0)
		setFollowMode(false);
	index = model()->index(max(0, index.row() + delta), 0);
	setCurrentIndex(index);
	scrollTo(model()->index(index.row(), 0));
}

bool LogView::eventFilter(QObject *target, QEvent *e)
{
	if (e->type() == QEvent::ScrollPrepare) {
		e = e;
	}
	
	return QTableView::eventFilter(target, e);;
}

LogItemDelegate::LogItemDelegate(QObject *parent):
	QStyledItemDelegate(parent)
{

}

QColor LogItemDelegate::getColorFromString(QString s, bool sameRGB) const
{
	QString hex = QCryptographicHash::hash(s.toStdString().c_str(),QCryptographicHash::Md5).toHex().toUpper().mid(0, 6);
	if(sameRGB)
		hex = hex.mid(0, 2) + hex.mid(0, 2) + hex.mid(0, 2);
	hex = "0x" + hex;
	bool ok;
	QRgb rgb = hex.toUInt(&ok, 16);
	QColor color(rgb);
	return color;
}

QColor CellColorizer::getColor(int column, const QString& text)
{
	auto qsum = [](const QString& s) -> quint32 
	{
		quint32 qsum_ = 0;
		foreach(auto ch, s.toStdString())
		{
			qsum_ += ch;
		}
		return qsum_;
	};
	auto getColorFromString = [](QString text, bool sameRGB) -> QColor
	{
		QString hex = QCryptographicHash::hash(text.toStdString().c_str(), QCryptographicHash::Md5).toHex().toUpper().mid(0, 6);
		if (sameRGB)
			hex = hex.mid(0, 2) + hex.mid(0, 2) + hex.mid(0, 2);
		hex = "0x" + hex;
		bool ok;
		QRgb rgb = hex.toUInt(&ok, 16);
		QColor color(rgb);
		return color;
	};

	QString textPart = text.mid(0, colorizeByChars);
	if (availableColors.size() == 0)
		return getColorFromString(textPart, false);
	int colorIdx = 0;
	if (alternateColors) {
		if (alternate.currentText != text) {
			alternate.currentColorIndex = (alternate.currentColorIndex + 1) % availableColors.size();
			alternate.currentText = text;			
		}
		return availableColors.at(alternate.currentColorIndex);
	}
	else {
		int colorIdx = qsum(textPart) % availableColors.size();
		return availableColors.at(colorIdx);
	}
}

void LogItemDelegate::paint(QPainter *painter,
	const QStyleOptionViewItem &option,
	const QModelIndex &index) const
{
	QVariant text = index.model()->data(index, Qt::DisplayRole);
	if (option.showDecorationSelected && (option.state & QStyle::State_Selected)) {
		if (option.state & QStyle::State_Active)
			painter->fillRect(option.rect, QColor(0xeeeeee));
		else {
			QPalette p = option.palette;
			painter->fillRect(option.rect, p.color(QPalette::Inactive, QPalette::Background));
		}
		painter->drawText(option.rect, option.displayAlignment, text.toString().split("\n").at(0));
		return;
	}
	if (true) {
		//QVariant text = index.model()->data(index, Qt::DisplayRole);
		if (text.type() == QVariant::DateTime) {
			text = text.toDateTime().toString("dd.MMM hh:mm:ss.zzz");
			//text = text.toDateTime().toString();
		}
		QStyleOptionViewItem myOption = option;
		//myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
		QBrush orgBrush = painter->brush();
		QPen savePen = painter->pen();
		LogView *view = qobject_cast<LogView *>(this->parent());
		auto qsum = [](const QString& s) -> quint32 {
			quint32 qsum_ = 0;
			foreach(auto ch, s.toStdString())
			{
				qsum_ += ch;
			}
			return qsum_;
		};
		if (view) {
			// colorize the forground depends on chars
			CellColorizer& colorizer = view->getRowStyle().getCellColorizer(index.column());
			if (colorizer.colorizeByChars) {
				QColor color = colorizer.getColor(index.column(), text.toString());
				QPen pen = painter->pen();
				pen.setColor(color);
				painter->setPen(pen);
			}

			RowColorizer& rColorizer = view->getRowStyle().getRowColorizer();
			// colrize the background
			if (rColorizer.boundColumn && rColorizer.colorizeByChars) {
				QModelIndex colIndex = index.model()->index(index.row(), rColorizer.boundColumn, index);
				auto colorizeByText = index.model()->data(colIndex).toString().mid(0, rColorizer.colorizeByChars);
				QColor color = rColorizer.getColor(index.column(), colorizeByText);
				painter->fillRect(myOption.rect, color);
			}
		}

		view->getRowStyle().textPartColorizer.drawText(painter, myOption, index.column(), text.toString().split("\n").at(0));
		//painter->drawText(myOption.rect, text);

		QRect r;
		//QString textLeft = text.toString();
		//myOption.palette.setColor(QPalette::HighlightedText, Qt::blue);
		if (false && index.row() == view->currentIndex().row()) {
			QColor bgColor = QColor(Qt::lightGray);
			painter->fillRect(myOption.rect, bgColor);
		}
		painter->setPen(savePen);
	} else{
		QPen pen(Qt::green, 30, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
		painter->setPen(pen);
		QStyleOptionViewItem newOption(option);
		//drawBackground(painter, option, index);

		QStyledItemDelegate::paint(painter, newOption, index);
	}
}

QString LogItemDelegate::displayText(const QVariant &value, const QLocale &) const
{
	if(value.type() == QVariant::DateTime) {
		return value.toDateTime().toString("dd.MMM hh:mm:ss.zzz");
	}
	return value.toString();
}

void LogItemDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.column() == 1) {//assume that id keeps in a second column
		const int id = index.data().toInt();
		painter->fillRect(option.rect, Qt::red);
	}
}

void TextPartColorizer::addText(const QString &textPart, QColor color, int column)
{
	column = -1;
	TColoredTextPart ctp;
	ctp.color = color;
	ctp.textPart = textPart;
	_coloredTextParts[textPart] = ctp;
}


void TextPartColorizer::removeText(const QString &textPart)
{
	_coloredTextParts.remove(textPart);
}

void TextPartColorizer::setFindText(const QString &textPart, QColor color, int column)
{
	_findCTP.color = color;
	_findCTP.textPart = textPart;
}

void TextPartColorizer::unsetFindText(const QString &textPart)
{
	_findCTP.color = QColor(0, 0, 0);
	_findCTP.textPart = "";
}

void TextPartColorizer::drawText(QPainter *painter, const QStyleOptionViewItem &option, int column, const QString &text)
{
	column = -1;
	if (_findCTP.textPart.length()) {
		_coloredTextParts["__find_text_part"] = _findCTP;
	}
	foreach(TColoredTextPart ctp, _coloredTextParts) {
		QString textLeft = text;
		int pos = textLeft.indexOf(ctp.textPart);
		QRect rFill = option.rect;
		while(pos >= 0) {
			QString textPart = textLeft.mid(0, pos);
			textLeft = textLeft.mid(pos);
			QRect rLeft = painter->boundingRect(option.rect, option.decorationAlignment, textPart);
			QRect rPart = painter->boundingRect(option.rect, option.decorationAlignment, ctp.textPart);
			rFill.setTop(rPart.top());
			rFill.setHeight(rPart.height());
			rFill.setLeft(rFill.left() + rLeft.width());
			rFill.setWidth(rPart.width());

			painter->fillRect(rFill, ctp.color);
			if(ctp.textPart.length() == 0)
				break;
			textLeft = textLeft.mid(ctp.textPart.length());
			pos = textLeft.indexOf(ctp.textPart);
		}
	}
	if (_findCTP.textPart.length())
		_coloredTextParts.remove("__find_text_part");
	painter->drawText(option.rect, option.displayAlignment, text);
	return;
}

TColoredTextPart *TextPartColorizer::getByText(const QString &text)
{
	if(_coloredTextParts.find(text) != _coloredTextParts.end())
		return &(*_coloredTextParts.find(text));
	return NULL;
}

LogView::LogView(QWidget *parent):
	QTableView(parent)
{
	//_QueryOptions = NULL;
	_findWidget = NULL;

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));

	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
	//QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::MoveToPreviousLine), this);
	connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));
	this->setAutoScroll(false);
	//verticalScrollBar()->installEventFilter(this);
	/*
	QHeaderView * header = horizontalHeader();
	for(int col = 0; col < header->count(); col++)
		header->setSectionResizeMode(col, QHeaderView::Stretch);
*/
	
	setFollowMode(false);
	setSelectionBehavior(SelectRows);
#ifdef USE_FROZENCOLUMNS
	initFrozenTable();
#endif
	setItemDelegate(new LogItemDelegate(this));
	_lastVerticalScrollPos = verticalScrollBar()->value();
	/*setStyleSheet("\
		QTableView::item:hover{\
			background-color: rgba(200, 200, 220, 255);\
		};\
		QTableView::item:selected{ \
			background - color: rgba(200, 200, 220, 255); \
		}");*/
	//setStyleSheet("LogView::item:selected{background-color: palette(highlight); color: palette(highlightedText);};");
}

LogView::~LogView()
{
	LogModel *m = qobject_cast<LogModel *>(model());
	m->removeView(this);
	qDebug()<<"~LogView()";
}


void LogView::doubleClicked(const QModelIndex& index)
{
	  LogModel *logModel = qobject_cast<LogModel *>(model());
	  QSqlRecord r = logModel->getColumnsInformation();

	  QString colName = r.fieldName(index.column());
	  Conditions qc = logModel->getQueryConditions();
	  QSqlField f;
	  f.setType(QVariant::String);
	  f.setValue(logModel->data(index).toString());

	  qc.queryString(colName + "='" + logModel->data(index).toString() + "'");
	  qc.limit(Settings().general().queryConditions().limit());

	  MainWindow::instance().createLogView(qc, this);
}

void LogView::testSlot(const QModelIndex &)
{
	qDebug() << "testSlot";
}

void LogView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
	log_trace(5)<<"dataChanged"<<topLeft.row();
}

void LogView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
	log_trace(5) << "currentChanged" << previous.row() << current.row();
}

void LogView::setModifiedPos(quint32 row)
{
	log_trace(5) << "setModifedPos" << row;
	//selectRow(row);
	//setCurrentIndex(index);
	//auto pos = horizontalScrollBar()->sliderPosition();
	//scrollTo(model()->index(row, 0), PositionAtTop);
	selectionModel()->setCurrentIndex(model()->index(row, 0), QItemSelectionModel::NoUpdate);
	//horizontalScrollBar()->setSliderPosition(pos);
}

void LogView::verticalScrollbarAction(int action)
{
	log_trace(5) << action;
}

void LogView::verticalScrollbarValueChanged(int value)
{
	log_join(logger::Logger::Level::Debug) << "change" << _lastVerticalScrollPos << value;
	if (value == _lastVerticalScrollPos)
		return;
	
	bool down = value > _lastVerticalScrollPos;	
	//log_trace(5) << "change" << _lastVerticalScrollPos << "to" << value;
	_lastVerticalScrollPos = value;
	auto visibleRows = verticalScrollBar()->pageStep();
	int fetched = 0;
	if (value == verticalScrollBar()->minimum()) {
		model()->fetchToBegin();
		emit model()->layoutChanged();
		return;
	}
	else if (value == verticalScrollBar()->maximum()) {
		model()->fetchToEnd();
		emit model()->layoutChanged();
		return;
	}
	else if (value > verticalScrollBar()->minimum() + visibleRows
			 && value < verticalScrollBar()->maximum() - visibleRows) {
		//log_trace(5) << "scrollbar value=" << value << "fetched=" << fetched;
		model()->rowCount();
	}
	else if (down == true && value >= verticalScrollBar()->maximum() - visibleRows) {
		blockSignals(true);
		fetched = model()->fetchMoreFrom(value, visibleRows, true);
		log_trace(5) << "scrollbar value=" << value << "fetched=" << fetched;
		verticalScrollBar()->blockSignals(true);
		verticalScrollBar()->setValue(value - fetched);
		_lastVerticalScrollPos = value - fetched;

		verticalScrollBar()->blockSignals(false);
		QModelIndex index = model()->index(currentIndex().row() - fetched, currentIndex().column());
		//setCurrentIndex(index);
		blockSignals(false);
		log_trace(5) << "change done " << currentIndex().row() << verticalScrollBar()->value();
		repaint();
		emit model()->layoutChanged();
	} else if (down == false && value < verticalScrollBar()->minimum() + visibleRows) {
		blockSignals(true);
		fetched = model()->fetchMoreFrom(value, visibleRows, false);
		log_trace(5) << "scrollbar value=" << value << "fetched=" << fetched;
		verticalScrollBar()->blockSignals(true);
		verticalScrollBar()->setValue(value + fetched);
		_lastVerticalScrollPos = value + fetched;

		verticalScrollBar()->blockSignals(false);
		QModelIndex index = model()->index(currentIndex().row() + fetched, currentIndex().column());
		//setCurrentIndex(index);
		blockSignals(false);
		log_trace(5) << "change done " << currentIndex().row() << verticalScrollBar()->value();
		repaint();
		emit model()->layoutChanged();
	}
}

#if 0
void LogView::setAvailableCellColors(const ColorList& colors, int col)
{
	_availableCellColors[col] = colors;
	emit model()->layoutChanged();
}

void LogView::setAvailableRowColors(const ColorList& colors)
{
	_availableRowColors = colors;
	emit model()->layoutChanged();
}
#endif
void LogView::writeSettings(const QString &basePath)
{
	Settings s(basePath);
	for(int col = 0; col < model()->columnCount(); col++) {
		s.column(col).width(columnWidth(col));
		s.column(col).visible(isColumnHidden(col) == false);
		s.column(col).colorize(_rowStyle.getCellColorizer(col).colorizeByChars);
		s.column(col).availableCellColors(_rowStyle.getCellColorizer(col).availableColors);
	}
	s.view().followMode(followMode());
	s.view().font(_rowStyle.font);
	s.view().fontSize(_rowStyle.font.pointSize());
	//s.view().alternatingRowColors(alternatingRowColors());
	s.view().rowStyle().alternatingRowColors(_rowStyle.alternateRowColors);
	
	s.view().availableRowColors(_rowStyle.getRowColorizer().availableColors);
	int idx = 0;
	foreach(TColoredTextPart ctp, _rowStyle.textPartColorizer.getList()) {
		s.view().rowStyle().textColorizer(idx).textPart(ctp.textPart);
		s.view().rowStyle().textColorizer(idx).textPartColor(ctp.color);
		idx++;
	}

	LogModel *logModel = qobject_cast<LogModel *>(model());
	if(logModel) {
		logModel->writeSettings(basePath);
	}

}

void LogView::readSettings(const QString &basePath)
{
	Settings s(basePath);

	foreach(QString colStr, s.childGroups("column")) {
		int col = colStr.toInt();
		if(s.column(col).width() > 0)
			setColumnWidth(col, s.column(col).width());
		setColumnHidden(col, s.column(col).visible() == false);
		_rowStyle.getCellColorizer(col).colorizeByChars = s.column(col).colorize();
		_rowStyle.getCellColorizer(col).availableColors = s.column(col).availableCellColors();
	}

	setFollowMode(s.view().followMode());

	
	QFont f = s.view().font();
	f.setPointSize(s.view().fontSize());
	_rowStyle.font = f;
	setFont(f);
	_rowStyle.alternateRowColors = s.view().rowStyle().alternatingRowColors();
	_rowStyle.getRowColorizer().availableColors = s.view().availableRowColors();
	foreach(QString group, s.childGroups("view/rowStyle/textColorizer")) {
		getRowStyle().textPartColorizer.addText(
			s.view().rowStyle().textColorizer(group).textPart(),
			s.view().rowStyle().textColorizer(group).textPartColor()
		);
	}

	int idx = 0;
	foreach(TColoredTextPart ctp, _rowStyle.textPartColorizer.getList()) {
		s.view().rowStyle().textColorizer(idx).textPart(ctp.textPart);
		s.view().rowStyle().textColorizer(idx).textPartColor(ctp.color);
		idx++;
	}
}

void LogView::setFollowMode(bool enabled)
{
	_followMode = enabled;
	if(enabled)
		scrollToBottom();
}
#if 0
void LogView::setAlternateRowColors(bool enabled)
{
	QTableView::setAlternatingRowColors(enabled);
}
#endif
void LogView::dataChanged()
{
	if(followMode())
		scrollToBottom();
}

void LogView::showFindWidget(bool show)
{
	LogModel *logModel = qobject_cast<LogModel *>(model());
	if(_findWidget == NULL) {
		_findWidget = new FindWidget(this);
		_findWidget->setBackgroundRole(horizontalHeader()->backgroundRole());
		//findWidget->setStyleSheet(horizontalHeader()->styleSheet());
		if(currentIndex().isValid()) {
			QString preferredColName = logModel->columns().at(currentIndex().column());
			foreach(QString colName, logModel->columns()) {
				_findWidget->addSearchField(colName, colName == preferredColName);
			}
		}
		//_findWidget->setFields(logModel->columns());
		connect(_findWidget, SIGNAL(find(QStringList, QString, bool, bool)), this, SLOT(find(QStringList, QString, bool, bool)));
		connect(_findWidget, SIGNAL(setFindTextColor(QStringList, const QString&)), this, SLOT(setFindTextColor(QStringList, QString)));
		QPoint p;
		p.setX(viewport()->pos().x() + viewport()->width() - _findWidget->width());
		p.setY(this->viewport()->pos().y());
		_findWidget->move(p);
	}
	if(show) {
		_findWidget->show();
		_findWidget->setFocus();
	} else
		_findWidget->hide();
}

void LogView::sliderMoved(int action)
{
	qDebug() << verticalScrollBar()->value() << "-" << verticalScrollBar()->maximum() << verticalScrollBar()->isMaximized();
	setFollowMode(verticalScrollBar()->value() == verticalScrollBar()->maximum());
}

void LogView::updateHeader()
{
	QHeaderView * header = horizontalHeader();
	if(header->count() > 0) {
		header->setStretchLastSection(true);
	}
}

bool LogView::followMode() const
{
	return _followMode;
}

#if 0
int LogView::getColorizeColumn(int index) const
{
	return _coloredColumns.find(index) != _coloredColumns.end() ? _coloredColumns[index] : 0;
}


TColoredRowBackground LogView::getColorizeRow() const
{
	return _coloredRows;
}

TextPartColorizer& LogView::getColorizer()
{
	return _textPartColorizer;
}

ColorList emptyColorList;
ColorList LogView::getAvailableCellColors(int index) const
{	
	return _availableCellColors.find(index) == _availableCellColors.end() ? emptyColorList : _availableCellColors[index];
}
#endif
class LogViewContextMenu : public QMenu
{
public:
	LogViewContextMenu(QWidget *parent) :
		QMenu(parent)
	{

	}

	void hideEvent(QHideEvent *e)
	{
		QMenu::hideEvent(e);
		show();
		
		qDebug() << "hide";
	}
	void mouseReleaseEvent(QMouseEvent *e)
	{
		QAction *action = activeAction();
		if (action && action->isEnabled()) {
			action->setEnabled(false);
			QMenu::mouseReleaseEvent(e);
			action->setEnabled(true);
			action->trigger();
		}
		else
			QMenu::mouseReleaseEvent(e);
	}
};
#include <qspinbox.h>
void LogView::contextMenu(const QPoint& point)
{
	QModelIndex index = indexAt(point);

	QMenu menu(this);
	LogModel *logModel = qobject_cast<LogModel *>(model());
	QAction *action;
	QWidgetAction *widgetAction;

	action = new QAction(tr("FollowMode"), &menu);
	action->setCheckable(true);
	action->setChecked(followMode());
	connect(action, SIGNAL(toggled(bool)), this, SLOT(setFollowMode(bool)));
	menu.addAction(action);


	action = new QAction(tr("Find"), &menu);
	connect(action, SIGNAL(triggered()), this, SLOT(showFindWidget()));
	menu.addAction(action);


	ContextMenuFilterDialog *filterWidget = new ContextMenuFilterDialog(&menu);
	if(logModel) {
		filterWidget->select("");
		filterWidget->filter(logModel->getQueryConditions().queryString());
		filterWidget->limit(logModel->getQueryConditions().limit());
		connect(filterWidget, SIGNAL(query(QString,int)), this, SLOT(queryWithCondition(QString,int)));
	}
	widgetAction = new QWidgetAction(&menu);
	widgetAction->setDefaultWidget(filterWidget);
	menu.addMenu(tr("Filter"))->addAction(widgetAction);


	RowLayoutWidget *rowLayoutWidget = new RowLayoutWidget(&menu, _rowStyle, index);
	connect(rowLayoutWidget, &RowLayoutWidget::rowStyleChanged, this, [&](const RowStyle& rowStyle) {
		setAlternatingRowColors(rowStyle.alternateRowColors);
		setFont(rowStyle.font);

	});

#if 0
	//rowLayoutWidget->setRowStyle
	SMapInt_IntInt smColorize;

	SMapInt_IntInt smColorizeRow;
	SMapInt_IntInt smLigthnessRow;
	if(logModel) {
		//rowLayoutWidget->setAvailableCellColors(getAvailableCellColors(index.column()));
		//rowLayoutWidget->setAvailableRowColors(getAvailableRowColors());
		rowLayoutWidget->getColorizeColumnSpin()->setValue(getColorizeColumn(index.column()));
		rowLayoutWidget->getLightnessSpin()->setValue(getColorizeRow().ligthness);


		connect(rowLayoutWidget->getColorizeColumnSpin(), (void (QSpinBox::*)(int))&QSpinBox::valueChanged, [&](int colorizeByChars) {
			this->setColorizeColumn(colorizeByChars, index.column());
		});
		connect(rowLayoutWidget->getColorizeRowSpin(), (void (QSpinBox::*)(int))&QSpinBox::valueChanged, [&](int colorizeByChars) {
			this->setColorizeRow(colorizeByChars, index.column());
		});
		/*smColorize.prm1 = index.column();
		smColorize.docon(rowLayoutWidget->getColorizeColumnSpin(),
			SIGNAL(valueChanged(int)), this, SLOT(setColorizeColumn(int,int)));*/
		rowLayoutWidget->getColorizeRowSpin();

		smColorizeRow.prm1 = index.column();
		smColorizeRow.docon(rowLayoutWidget->getColorizeRowSpin(),
			SIGNAL(valueChanged(int)), this, SLOT(setColorizeRow(int,int)));

		//rowLayoutWidget->setAvailableCellColors(_availableCellColors);
		//rowLayoutWidget->setAvailableRowColors(_availableRowColors);
		//connect(rowLayoutWidget, &RowLayoutWidget::CellColorChanged)
		
		smLigthnessRow.prm1 = index.column();
		smLigthnessRow.docon(rowLayoutWidget->getLightnessSpin(),
			SIGNAL(valueChanged(int)), this, SLOT(setLigthnessRow(int,int)));
		QRect r;
		QString text = model()->data(index).toString();
		/*
		foreach(QChar ch, text) {
			r.setWidth(r.width() + this->paintEngine()->painter()->boundingRect(option.rect, option.displayAlignment, ch));
			if(point.x() < r.width())
				break;

		}*/
		QTextCursor tc;

		tc.select(QTextCursor::WordUnderCursor);

		QString word = tc.selectedText();

		foreach(TColoredTextPart part, _textPartColorizer.getList()) {
			rowLayoutWidget->getTextPartColorCombo()->addItem(part.textPart);
			rowLayoutWidget->getTextPartColorSlider()->setValue(part.color.value());
		}


	}
	rowLayoutWidget->getAlternateRowColorCheck()->setChecked(alternatingRowColors());
	connect(rowLayoutWidget->getAlternateRowColorCheck(), SIGNAL(toggled(bool)), this, SLOT(setAlternateRowColors(bool)));

	rowLayoutWidget->getFontCombo()->setFont(font());
	connect(rowLayoutWidget->getFontCombo(), SIGNAL(currentFontChanged(QFont)), this, SLOT(setFont(QFont)));
	rowLayoutWidget->getFontSizeSpin()->setValue(font().pointSize());
	connect(rowLayoutWidget->getFontSizeSpin(), SIGNAL(valueChanged(int)), this, SLOT(setFontSize(int)));
#endif
	widgetAction = new QWidgetAction(&menu);
	widgetAction->setDefaultWidget(rowLayoutWidget);
	menu.addMenu(tr("Style"))->addAction(widgetAction);


	widgetAction = new QWidgetAction(&menu);
	QColorDialog *d = new QColorDialog(&menu);
	widgetAction->setDefaultWidget(d);
	menu.addMenu(tr("Colors"))->addAction(widgetAction);
	//action = new QAction(tr("Colors"), &menu);
	

	action = new QAction(tr("Alternate row colors"), &menu);
	action->setCheckable(true);
	action->setChecked(alternatingRowColors());
	connect(action, SIGNAL(toggled(bool)), this, SLOT(setAlternateRowColors(bool)));
	menu.addAction(action);

	QMenu *visbleColumnsMenu = menu.addMenu(tr("Visible columns"));
	QSqlRecord cols = logModel->getColumnsInformation();

	for(int col = 0; col < logModel->columnCount(); col++) {
		QCheckBox *checkBox = new QCheckBox(&menu);
		checkBox->setText(cols.fieldName(col));
		checkBox->setChecked(isColumnHidden(col) == false);
		QWidgetAction *checkableAction = new QWidgetAction(&menu);
		checkableAction->setDefaultWidget(checkBox);
		visbleColumnsMenu->addAction(checkableAction);
		SMapBool_BoolInt *sm = new SMapBool_BoolInt(&menu);
		sm->prm1 = col;
		sm->docon(checkBox, SIGNAL(toggled(bool)), this, SLOT(showColumn(bool,int)));
	}
	menu.exec(viewport()->mapToGlobal(point));

	log_debug() << "done contextmenu";
}

int LogView::find(const QStringList& columns, const QString& search, bool regex, bool down)
{
	QModelIndex index = model()->find(currentIndex(), columns, search, regex, down);
	qDebug()<<"index.row="<<index.row()<<" index.col="<<index.column();
	if(true ||index.isValid()) {
		selectRow(index.row());
		setCurrentIndex(index);
		auto pos = horizontalScrollBar()->sliderPosition();
		scrollTo(index, EnsureVisible);
		horizontalScrollBar()->setSliderPosition(pos);
	}
	return index.row();

}

void LogView::showColumn(bool show, int col)
{
	qDebug() << "showColumn("<<show<<","<<col<<")";
	if(show)
		QTableView::showColumn(col);
	else
		hideColumn(col);
}
#if 0
void LogView::setColorizeColumn(int colorize, int col)
{
	_coloredColumns[col] = colorize;
	emit model()->layoutChanged();
	return;
}

void LogView::setColorizeRow(int colorize, int row)
{
	_coloredRows.colorizeByChars = colorize;
	_coloredRows.column = row;
	emit model()->layoutChanged();
}

void LogView::setLigthnessRow(int ligthness, int row)
{
	_coloredRows.ligthness = ligthness;
	_coloredRows.column = row;
	emit model()->layoutChanged();
}

void LogView::setTextPartColor(const QString &text, int color)
{
	if(color == 0) {
		_textPartColorizer.removeText(text);
		return;
	}
	QColor col(color);
	_textPartColorizer.addText(text, color);
}
#endif
void LogView::setFindTextColor(const QStringList& columns, const QString& text)
{
	_rowStyle.textPartColorizer.setFindText(text, QColor(155, 144, 22));
	emit model()->layoutChanged();
	//emit updateLayout;
	qDebug() << "setFindTextColor:" << text;
}

bool LogView::queryWithCondition(QString filter, int limit)
{
	qDebug() << "QueryOptions:"<<filter<<","<<limit;
	LogModel *logModel = qobject_cast<LogModel *>(model());
	if(logModel->queryWithCondition(filter, limit) == false)
		return false;
	LogWindow *wnd = qobject_cast<LogWindow *>(parentWidget());
	wnd->refreshTitle();
	return true;
}

LogWindow::LogWindow()
{
	//_logModel = new LogModel(NULL);
	setOrientation(Qt::Vertical);
	_logView = new LogView(this);
	_detailView = new DetailView(this);
	//setModel(_logModel);
	_logView->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	_logView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

LogWindow *LogWindow::create(Conditions qc, bool useTemplate)
{
	LogWindow *window = new LogWindow();
	LogModel *model = nullptr;
	try {
		if (qc.modelClass().length()) {
			if (qc.modelClass() == "LogSqlModel")
				model = new LogModel(NULL);
			else if (qc.modelClass() == "LogFileModel")
				model = new LogFileModel(NULL);
			else
				throw std::exception("invalid modelClass");
			model->setQueryConditions(qc);
		}
		else
			throw std::exception("invalid modelClass");
		window->setModel(model);
		if (window->query(model->getQueryConditions()) == false) {
			delete window;
			return nullptr;
		}
	}
	catch (std::exception&) {
		if (window)
			delete window;
		if (model)
			delete model;
		throw;
	}

	if(useTemplate) {
		Settings s;
		foreach(const QString& group, s.childGroups("windowTemplates")) {
			QString filter = s.windowTemplates(group).viewNameFilter();
			QRegExp regex(filter);
			if(model->getTitle().contains(regex))
				window->readSettings(s.windowTemplates(group).logView().getPath());
		}
	}
	
	window->refreshTitle();
	return window;
}

LogWindow *LogWindow::create(Conditions qc, LogView *templateView)
{
	auto window = create(qc, false);
	if (!window)
		return nullptr;

	// now use the present settings technoligy to move needed windows settings
	Settings s;
	QString tmpName = "tmporary_template";
	templateView->writeSettings(s.windowTemplates(tmpName).logView().getPath());
	window->readSettings(s.windowTemplates(tmpName).logView().getPath());
	s.windowTemplates(tmpName).remove();
	return window;
}

LogWindow *LogWindow::create(const QString& settingsPath)
{
	Conditions qc;
	qc.readSettings(settingsPath);
	LogWindow *window = create(qc);
	window->readSettings(settingsPath);
	return window;
}

bool LogWindow::query(const Conditions& qc)
{
	_logModel->query(qc);
	refreshTitle();
	return true;
}

void LogWindow::refreshTitle()
{
	QString title = _logModel->getTitle();
	setWindowTitle(title);
	if(this->isActiveWindow()) {
		MainWindow::instance().refreshWindowTitle();
	}
	_logView->updateHeader();
}

void LogWindow::setModel(LogModel *model)
{
	//QAbstractItemModel *itemModel = dynamic_cast<QAbstractItemModel *>(model);
	if(model == NULL)
		return;
	model->addView(_logView);
	_logModel = model;
	_logView->setModel(model);
	_detailView->setModel(model);
	connect(model, SIGNAL(layoutChanged()), _logView, SLOT(dataChanged()));
	connect(model, SIGNAL(setModifiedPos(quint32)), _logView, SLOT(setModifiedPos(quint32)));
	connect(_logView->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
			_detailView, SLOT(currentRowChanged(QModelIndex,QModelIndex)));
	//setTabKeyNavigation(true);
}

void LogWindow::writeSettings(const QString &basePath)
{
	Settings s(basePath);
	_logModel->writeSettings(basePath);
	_logView->writeSettings(basePath);
	_detailView->writeSettings(basePath);
	s.view().splitter(saveState());
}

void LogWindow::readSettings(const QString &basePath)
{
	Settings s(basePath);
	_logModel->readSettings(basePath);
	_logView->readSettings(basePath);
	_detailView->readSettings(basePath);
	restoreState(s.view().splitter());
}

void LogWindow::setFollowMode(bool enabled)
{
	_logView->setFollowMode(enabled);
}

void LogWindow::showFindWidget(bool show)
{
	_logView->showFindWidget(show);
}

DetailView::DetailView(QWidget *parent):
	QTextEdit(parent)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
}

void DetailView::writeSettings(const QString &basePath)
{
	Settings s(basePath);
	for(int col = 0; col < _logModel->columnCount(); col++)
		s.column(col).visibleDetail(_visibleColumns[col]);
}

void DetailView::readSettings(const QString &basePath)
{
	Settings s(basePath);
	foreach(QString colStr, s.childGroups("column")) {
		int col = colStr.toInt();
		_visibleColumns[col] = s.column(col).visibleDetail();
	}
}

void DetailView::contextMenu(const QPoint& point)
{
	//ContextMenuDetailView menu(this);
	QMenu menu(this);

	QMenu *visbleColumnsMenu = menu.addMenu(tr("Visible columns"));
	QSqlRecord cols = _logModel->getColumnsInformation();
	for(int col = 0; col < _logModel->columnCount(); col++) {
		QCheckBox *checkBox = new QCheckBox(&menu);
		checkBox->setText(cols.fieldName(col));
		checkBox->setChecked(_visibleColumns[col]);
		QWidgetAction *checkableAction = new QWidgetAction(&menu);
		checkableAction->setDefaultWidget(checkBox);
		visbleColumnsMenu->addAction(checkableAction);
		SMapBool_BoolInt *sm = new SMapBool_BoolInt(&menu);
		sm->prm1 = col;
		sm->docon(checkBox, SIGNAL(toggled(bool)), this, SLOT(showColumn(bool,int)));
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
	qDebug()<<"currentRowChanged "<<current<<","<<last;
	/*
	 * @todo make detail view of multiple selectsions
	QItemSelectionModel *selected = this->selectionModel();
	*/
	for(int col = 0; col < _logModel->columnCount(); col++) {
		if(_visibleColumns[col]) {
			out += _logModel->data(current.row(), col, Qt::DisplayRole).toString() + "<br>";
			
		}		
	}
	out = utils::ReplaceAll(out.toStdString(), "\n", "<br>").c_str();
	out = utils::ReplaceAll(out.toStdString(), " ", "&nbsp;").c_str();
	//QVariant v = _logModel->data(current, Qt::DisplayRole);
	this->setText(out);
	//this->setHtml(out);
}


#endif
