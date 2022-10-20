#include <gui/logview/logview.h>
#include <gui/logview/ColumnToolTip.h>
//#include "gui/LogWIndow/mainwindow.h"
#if 1
#include <core/signalmapper.h>

#include <gui/finddialog.h>
#include <gui/rowlayoutwidget.h>
#include <gui/fontstylewidget.h>
#include <gui/contextmenu.h>
#include <gui/contextmenufilterdialog.h>
#include <gui/TextColorizerWidget.h>

#include <QSqlField>
#include <QTWidgets>
#include <QDebug>
#include <QSignalMapper>
#include <QKeySequence>
#include <QKeyEvent>
#include <QKeyEventTransition>
#include <gui\MainWindow.h>

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
	index = model()->index(std::max(0, index.row() + delta), 0);
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

LogView::LogView(QWidget *parent):
	QTableView(parent)
{
	_findWidget = NULL;

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));

	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
	connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));
	this->setAutoScroll(false);
	setSelectionBehavior(SelectRows);
	setItemDelegate(new LogItemDelegate(this));
	_lastVerticalScrollPos = verticalScrollBar()->value();
	horizontalHeader()->setSectionsMovable(true);
	resizeColumnsToContents();
	viewport()->installEventFilter(new ColumnToolTip(this));
}

LogView::~LogView()
{
}


void LogView::doubleClicked(const QModelIndex& index)
{
	  LogModel *logModel = qobject_cast<LogModel *>(model());
	  QSqlRecord r = logModel->columnsInformation();

	  QString colName = r.fieldName(index.column());
	  
	  QSqlField f;
	  f.setType(QVariant::String);
	  f.setValue(logModel->data(index).toString());

	  LogWindow::CreateParams params;
	  params.templateWindow = qobject_cast<LogWindow *>(this->parent());
	  params.queryParams = logModel->getQueryParams();
	  params.queryParams->queryString(colName + ":" + logModel->data(index).toString());
	  //qc.limit(settings.general().queryConditions().limit());
	  emit MainWindow::instance().createLogWindow(params);
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
	selectionModel()->setCurrentIndex(model()->index(row, 0), QItemSelectionModel::NoUpdate);
}

void LogView::verticalScrollbarAction(int action)
{
}

void LogView::verticalScrollbarValueChanged(int value)
{
	if (model()->virtualRows() == false)
		return;
	//return;
	//log_join(logger::Logger::Level::Debug) << "change" << _lastVerticalScrollPos << value;
	if (value == _lastVerticalScrollPos)
		return;
	
	bool down = value > _lastVerticalScrollPos;	
	//log_trace(5) << "change" << _lastVerticalScrollPos << "to" << value;
	_lastVerticalScrollPos = value;
	auto min = verticalScrollBar()->minimum();
	auto max = verticalScrollBar()->maximum();// +verticalScrollBar()->pageStep();
	auto visibleRows = verticalScrollBar()->pageStep();
	int fetched = 0;
	
	if (value == min) {
		//verticalScrollBar()->blockSignals(true);
		model()->fetchToBegin();				
		//emit model()->layoutChanged();
		//verticalScrollBar()->blockSignals(false);
		//emit verticalScrollBar()->setValue(0);
		return;
	}
	else if (value == verticalScrollBar()->maximum()) {
		model()->fetchToEnd();
		//emit model()->layoutChanged();
		//return;
	}
	else if (value > min + visibleRows && value < max - visibleRows) {
		//log_trace(5) << "scrollbar value=" << value << "fetched=" << fetched;
		model()->rowCount();
		return;
	}
	else if (down == true && value >= max - visibleRows) {
		blockSignals(true);
		fetched = model()->fetchMoreDownward(value, visibleRows);
		log_trace(5) << "scrollbar value=" << value << "fetched=" << fetched;
		//verticalScrollBar()->blockSignals(true);
		//verticalScrollBar()->setValue(value - fetched);
		_lastVerticalScrollPos = value + fetched;

		//verticalScrollBar()->blockSignals(false);
		QModelIndex index = model()->index(currentIndex().row() - fetched, currentIndex().column());
		//setCurrentIndex(index);
		blockSignals(false);
		log_trace(5) << "change done " << currentIndex().row() << verticalScrollBar()->value();
		//repaint();
		//emit model()->layoutChanged();
	} else if (down == false && value < min + visibleRows) {
		blockSignals(true);
		fetched = model()->fetchMoreUpward(value, visibleRows);
		log_trace(5) << "scrollbar value=" << value << "fetched=" << fetched;
		//verticalScrollBar()->blockSignals(true);
		//verticalScrollBar()->setValue(value + fetched);
		_lastVerticalScrollPos = value - fetched;

		//verticalScrollBar()->blockSignals(false);
		QModelIndex index = model()->index(currentIndex().row() + fetched, currentIndex().column());
		//setCurrentIndex(index);
		blockSignals(false);
		log_trace(5) << "change done " << currentIndex().row() << verticalScrollBar()->value();
		//repaint();
		//emit model()->layoutChanged();
	}

	verticalScrollBar()->blockSignals(true);
	//repaint();
	emit model()->layoutChanged();
	verticalScrollBar()->blockSignals(false);
	emit verticalScrollBar()->setValue(_lastVerticalScrollPos);

}

void LogView::writeSettings(_settings::LogWindow& s)
{	
	for(int col = 0; col < model()->columnCount(); col++) {
		s.columns(col).width(columnWidth(col));
		s.columns(col).visible(isColumnHidden(col) == false);
		s.columns(col).colorize(_rowStyle.getCellColorizer(col).colorizeByChars);
		s.columns(col).availableCellColors(_rowStyle.getCellColorizer(col).availableColors);
	}
	s.followMode(followMode());
	s.font(font());
	s.header(horizontalHeader()->saveState());

	s.rowStyle().alternatingRowColors(_rowStyle.alternateRowColors);
	
	s.availableRowColors(_rowStyle.getRowColorizer().availableColors);
	int idx = 0;

	getRowStyle().textColorizer.writeSettings(s.rowStyle());

	LogModel *logModel = qobject_cast<LogModel *>(model());
	if(logModel) {
		logModel->writeSettings(s);
	}

}

void LogView::readSettings(_settings::LogWindow& s)
{
	for(auto it: s.columnsList()) {
		auto column = it.second;
		auto col = it.first.toInt();
		if(column.width() > 0)
			setColumnWidth(col, column.width());
		if (column.visible() == false) {
			hideColumn(col);
		}
		//setColumnHidden(col, s.column(col).visible() == false);
		_rowStyle.getCellColorizer(col).colorizeByChars = column.colorize();
		_rowStyle.getCellColorizer(col).availableColors = column.availableCellColors();
	}

	setFollowMode(s.followMode());

	
	//QFont f = s.font();
	//f.setPointSize(s.fontSize());
	setFont(s.font());
	_rowStyle.font = font();
	//_rowStyle.fontSize = s.fontSize();
	_rowStyle.alternateRowColors = s.rowStyle().alternatingRowColors();
	_rowStyle.getRowColorizer().availableColors = s.availableRowColors();

	getRowStyle().textColorizer.readSettings(s.rowStyle());
}

void LogView::setFollowMode(bool enabled)
{
	MainWindow::instance().updateMenu();
	_followMode = enabled;
	if(enabled)
		scrollToBottom();
	model()->followTail(enabled);
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
	//qDebug() << verticalScrollBar()->value() << "-" << verticalScrollBar()->maximum() << verticalScrollBar()->isMaximized();
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
		filterWidget->filter(logModel->getQueryParams().queryString());
		filterWidget->limit(logModel->getQueryParams().limit());
		connect(filterWidget, SIGNAL(query(QString,int)), this, SLOT(queryWithCondition(QString,int)));
	}
	widgetAction = new QWidgetAction(&menu);
	widgetAction->setDefaultWidget(filterWidget);
	menu.addMenu(tr("Filter"))->addAction(widgetAction);


	RowLayoutWidget *rowLayoutWidget = new RowLayoutWidget(&menu, _rowStyle, index);
	connect(rowLayoutWidget, &RowLayoutWidget::rowStyleChanged, this, [&](const RowStyle& rowStyle) {
		setAlternatingRowColors(rowStyle.alternateRowColors);
		setFont(rowStyle.font);
		//setFontSize(rowStyle.fontSize);
		//writeSettings(_settingsPath);
	});

	widgetAction = new QWidgetAction(&menu);
	widgetAction->setDefaultWidget(rowLayoutWidget);
	menu.addMenu(tr("Style"))->addAction(widgetAction);

	/*
	widgetAction = new QWidgetAction(&menu);
	auto d = new TextColorizerWidget(&menu);
	widgetAction->setDefaultWidget(d);
	menu.addMenu(tr("TextHighlighter"))->addAction(widgetAction);
	//action = new QAction(tr("Colors"), &menu);
	*/

	action = new QAction(tr("Alternate row colors"), &menu);
	action->setCheckable(true);
	action->setChecked(alternatingRowColors());
	connect(action, SIGNAL(toggled(bool)), this, SLOT(setAlternateRowColors(bool)));
	menu.addAction(action);

	QMenu *visbleColumnsMenu = menu.addMenu(tr("Visible columns"));
	QSqlRecord cols = logModel->columnsInformation();

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
	_rowStyle.textColorizer.setFindText(text, QColor(155, 144, 22));
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
#endif
