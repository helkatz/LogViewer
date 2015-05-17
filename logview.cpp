#include "logview.h"
#include "logsqlmodel.h"
#include "forms/contextmenu.h"
#include "forms/contextmenufilterdialog.h"
#include "mainwindow.h"
#include "forms/finddialog.h"
#include "signalmapper.h"
#include "ui_findwidget.h"
#include "forms/rowlayoutwidget.h"
#include "forms/fontstylewidget.h"
#include <QTWidgets>
#include <QDebug>
#include <QSignalMapper>
#include <QKeySequence>
#include <QKeyEvent>
#include <QKeyEventTransition>
class LogSqlModel;
#ifdef _DEBUG
#define connect( ... ) Q_ASSERT( connect( __VA_ARGS__ ) )
#endif
#ifdef USE_FROZENCOLUMNS
void LogView::initFrozenTable()
{
    frozenTableView = new QTableView(this);
    frozenTableView->setModel(model());
    frozenTableView->setFocusPolicy(Qt::NoFocus);
    frozenTableView->verticalHeader()->hide();
    frozenTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    frozenTableView->setSelectionBehavior(selectionBehavior());
    viewport()->stackUnder(frozenTableView);

    //connect the headers and scrollbars of both tableviews together
    connect(horizontalHeader(),SIGNAL(sectionResized(int,int,int)), this,
            SLOT(updateSectionWidth(int,int,int)));
    connect(verticalHeader(),SIGNAL(sectionResized(int,int,int)), this,
            SLOT(updateSectionHeight(int,int,int)));

    connect(frozenTableView->verticalScrollBar(), SIGNAL(valueChanged(int)),
            verticalScrollBar(), SLOT(setValue(int)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            frozenTableView->verticalScrollBar(), SLOT(setValue(int)));
    frozenTableView->setStyleSheet("QTableView { border: none;"
                                          "background-color: #8EDE21;"
                                          "selection-background-color: #999}"); //for demo purposes
   frozenTableView->setSelectionModel(selectionModel());
   for(int col=1; col<model()->columnCount(); col++)
         frozenTableView->setColumnHidden(col, true);

   frozenTableView->setColumnWidth(0, columnWidth(0) );

   frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   frozenTableView->show();

   updateFrozenTableGeometry();

   frozenTableView->setVerticalScrollMode(verticalScrollMode());
}

void LogView::updateSectionWidth(int logicalIndex, int, int newSize)
{
      if(logicalIndex==0){
            frozenTableView->setColumnWidth(0,newSize);
            updateFrozenTableGeometry();
      }
}

void LogView::updateSectionHeight(int logicalIndex, int, int newSize)
{
      frozenTableView->setRowHeight(logicalIndex, newSize);
}

void LogView::resizeEvent(QResizeEvent * event)
{
      QTableView::resizeEvent(event);
      updateFrozenTableGeometry();
}

QModelIndex LogView::moveCursor(CursorAction cursorAction,
                                          Qt::KeyboardModifiers modifiers)
{
      QModelIndex current = QTableView::moveCursor(cursorAction, modifiers);

      if(cursorAction == MoveLeft && current.column()>0
         && visualRect(current).topLeft().x() < frozenTableView->columnWidth(0) ){

            const int newValue = horizontalScrollBar()->value() + visualRect(current).topLeft().x()
                                 - frozenTableView->columnWidth(0);
            horizontalScrollBar()->setValue(newValue);
      }
      return current;
}

void LogView::updateFrozenTableGeometry()
{
      frozenTableView->setGeometry( verticalHeader()->width()+frameWidth(),
                                    frameWidth(), columnWidth(0),
                                    viewport()->height()+horizontalHeader()->height());
}

#endif // USE_FROZENCOLUMNS
void LogView::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Up:
        //scrollTo((0, -1);
        break;
    case Qt::Key_Down:
        //scroll(0, 1);
        break;
    }
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

void LogItemDelegate::paint(QPainter *painter,
    const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    if (true) {
        QVariant text = index.model()->data(index, Qt::DisplayRole);
        if(text.type() == QVariant::DateTime) {
            QString dt = text.toString();
            text = text.toDateTime().toString("dd.MMM hh:mm:ss.zzz");
        }
        QStyleOptionViewItem myOption = option;
        //myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        QBrush orgBrush = painter->brush();
        QPen savePen = painter->pen();
        LogView *view = qobject_cast<LogView *>(this->parent());

        if(view) {
            // colorize the forground depends on chars
            int colorizeByChars = view->getColorizeColumn(index.column());
            if(colorizeByChars) {
                QColor color = getColorFromString(text.toString().mid(0, colorizeByChars));

                QPen pen = painter->pen();
                pen.setColor(color);
                painter->setPen(pen);
                //QStyledItemDelegate::paint(painter, myOption, index);
                //return;
            }
            // colrize the background
            TColoredRowBackground rowBG = view->getColorizeRow();
            if(rowBG.column && rowBG.charsCount) {
                QModelIndex colIndex = index.model()->index(index.row(), rowBG.column, index);

                QString colText = index.model()->data(colIndex).toString();
                QColor bgColor = getColorFromString(colText.mid(0, rowBG.charsCount), false);
                QString hex = QCryptographicHash::hash(
                            colText.mid(0, rowBG.charsCount).toStdString().c_str(),
                            QCryptographicHash::Md5).toHex().toUpper().mid(0, 2);
                QMap<QString, int> colorMap;
                QString textPart = colText.mid(0, rowBG.charsCount);
                bool b;
                int ligther = (QString("0x%1").arg(hex)).toUInt(&b, 16);
                bgColor = bgColor.lighter(view->getColorizeRow().ligthness);
                painter->fillRect(myOption.rect, bgColor);
            }
        }

        view->getColorizer().drawText(painter, myOption, index.column(), text.toString().split("\n").at(0));
        //painter->drawText(myOption.rect, text);

        QRect r;
        //QString textLeft = text.toString();
        myOption.palette.setColor(QPalette::HighlightedText, Qt::blue);
        //QStyledItemDelegate::paint(painter, myOption, index);
        //painter->drawText(myOption.rect, myOption.displayAlignment, text.toString());
        painter->setPen(savePen);

        return;
        QRect rect = option.rect;
        QBrush brush(RGB(1,2,3));
        painter->setBackground(brush);
        //drawDisplay(painter, option, option.rect, "text1");
        r = painter->boundingRect(r, option.displayAlignment, "text1");
        rect.adjust(r.right(), 0, r.right(), 0);
        //drawDisplay(painter, option, rect, text.toString());
        //drawFocus(painter, myOption, option.rect);
    } else{
        QPen pen(Qt::green, 30, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(pen);
        QStyleOptionViewItem newOption(option);
        //drawBackground(painter, option, index);

        QStyledItemDelegate::paint(painter, newOption, index);
    }
}

QString LogItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
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

void TextPartColorizer::drawText(QPainter *painter, const QStyleOptionViewItem &option, int column, const QString &text)
{
    column = -1;
    int flags = 0;
    int idx = 0;
    struct TTextPart
    {
        QString text;
        QColor color;
    };
    QMap<int, TTextPart> textParts;
    foreach(TColoredTextPart ctp, _coloredTextParts) {
        QString textLeft = text;
        int pos = textLeft.indexOf(ctp.textPart);
        int curPos = 0;
        QRect rFill = option.rect;
        while(pos >= 0) {
            TTextPart part;
            part.text = textLeft.mid(0, pos);
            textLeft = textLeft.mid(pos);
            QRect rLeft = painter->boundingRect(option.rect, option.decorationAlignment, part.text);
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
    //_queryConditions = NULL;
    _findWidget = NULL;

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(doubleClicked(QModelIndex)));

    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
    //QShortcut* shortcut = new QShortcut(QKeySequence(QKeySequence::MoveToPreviousLine), this);
    connect(this->verticalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
    this->setAutoScroll(false);
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
    //verticalHeader()->setVisible(false);
    /*
    getColorizer().addText("sailing", Qt::red, 3);
    getColorizer().addText("corfu", Qt::blue, 3);
    getColorizer().addText("index", Qt::yellow, 3);
    getColorizer().addText("2014", Qt::yellow, 2);
    */
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

      MainWindow::instance().createLogView(qc, true);
}

void LogView::testSlot(const QModelIndex &)
{
    qDebug() << "testSlot";
}

void LogView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    qDebug()<<"dataChanged"<<topLeft;
}

void LogView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
}

void LogView::writeSettings(const QString &basePath)
{
    Settings s(basePath);
    for(int col = 0; col < model()->columnCount(); col++) {
        s.column(col).width(columnWidth(col));
        s.column(col).visible(isColumnHidden(col) == false);
        s.column(col).colorize(getColorizeColumn(col));
    }
    s.view().followMode(followMode());
    s.view().font(font());
    s.view().fontSize(font().pointSize());
    s.view().alternatingRowColors(alternatingRowColors());
    s.view().rowStyle().alternatingRowColors(alternatingRowColors());

    int idx = 0;
    foreach(TColoredTextPart ctp, getColorizer().getList()) {
        s.view().rowStyle().textColorizer(idx).textPart(ctp.textPart);
        s.view().rowStyle().textColorizer(idx).textPartColor(ctp.color);
        idx++;
    }

    LogSqlModel *logModel = qobject_cast<LogSqlModel *>(model());
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
        setColorizeColumn(s.column().colorize(), col);
    }

    setFollowMode(s.view().followMode());
    QFont f = s.view().font();
    f.setPointSize(s.view().fontSize());
    setFont(f);
    setAlternatingRowColors(s.view().rowStyle().alternatingRowColors());

    foreach(QString group, s.childGroups("view/rowStyle/textColorizer")) {
        getColorizer().addText(
            s.view().rowStyle().textColorizer(group).textPart(),
            s.view().rowStyle().textColorizer(group).textPartColor()
        );
    }

    int idx = 0;
    foreach(TColoredTextPart ctp, getColorizer().getList()) {
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

void LogView::setAlternateRowColors(bool enabled)
{
    QTableView::setAlternatingRowColors(enabled);
}

void LogView::dataChanged()
{
    if(followMode())
        scrollToBottom();
}

void LogView::showFindWidget(bool show)
{
    LogSqlModel *logModel = qobject_cast<LogSqlModel *>(model());
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
        connect(_findWidget, SIGNAL(find(QString,bool)), this, SLOT(find(QString,bool)));
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

void LogView::sliderMoved(int pos)
{
    setFollowMode(false);
}

void LogView::updateHeader()
{
    QHeaderView * header = horizontalHeader();
    if(header->count() > 0) {
        header->setStretchLastSection(true);
    }
}

void LogView::contextMenu(const QPoint& point)
{
    QModelIndex index = indexAt(point);

    QMenu menu(this);
    LogSqlModel *logModel = qobject_cast<LogSqlModel *>(model());
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

    RowLayoutWidget *rowLayoutWidget = new RowLayoutWidget(this);
    SMapInt_IntInt smColorize;

    SMapInt_IntInt smColorizeRow;
    SMapInt_IntInt smLigthnessRow;
    if(logModel) {
        rowLayoutWidget->getColorizeColumnSpin()->setValue(getColorizeColumn(index.column()));
        smColorize.prm1 = index.column();
        smColorize.docon(rowLayoutWidget->getColorizeColumnSpin(),
            SIGNAL(valueChanged(int)), this, SLOT(setColorizeColumn(int,int)));
        rowLayoutWidget->getColorizeRowSpin();

        smColorizeRow.prm1 = index.column();
        smColorizeRow.docon(rowLayoutWidget->getColorizeRowSpin(),
            SIGNAL(valueChanged(int)), this, SLOT(setColorizeRow(int,int)));

        rowLayoutWidget->getLightnessSpin()->setValue(getColorizeRow().ligthness);
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
    widgetAction = new QWidgetAction(&menu);
    widgetAction->setDefaultWidget(rowLayoutWidget);
    menu.addMenu(tr("Style"))->addAction(widgetAction);


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

    qDebug() << "done contextmenu";
}

int LogView::find(const QString& where, bool down)
{
    QModelIndex index = model()->find(currentIndex(), where, down);
    qDebug()<<"index.row="<<index.row()<<" index.col="<<index.column();
    if(index.isValid()) {
        selectRow(index.row());
        setCurrentIndex(index);
        scrollTo(index);
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

void LogView::setColorizeColumn(int colorize, int col)
{
    _coloredColumns[col] = colorize;
    emit model()->layoutChanged();
    return;
}

void LogView::setColorizeRow(int colorize, int row)
{
    _coloredRows.charsCount = colorize;
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

void LogView::queryWithCondition(QString filter, int limit)
{
    qDebug() << "queryConditions:"<<filter<<","<<limit;
    LogSqlModel *logModel = qobject_cast<LogSqlModel *>(model());
    logModel->queryWithCondition(filter, limit);
    LogWindow *wnd = qobject_cast<LogWindow *>(parentWidget());
    wnd->refreshTitle();
}

LogWindow::LogWindow()
{
    //_logModel = new LogSqlModel(NULL);
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
    LogModel* model = LogModel::create(qc);
    if(model == NULL)
        return NULL;
    window->setModel(model);

    if(useTemplate) {
        Settings s;
        foreach(const QString& group, s.childGroups("windowTemplates")) {
            QString filter = s.windowTemplates(group).viewNameFilter();
            QRegExp regex(filter);
            if(model->getTitle().contains(regex))
                window->readSettings(s.windowTemplates(group).logView().getPath());
        }
    }
    window->query(model->getQueryConditions());
    window->refreshTitle();
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

void LogWindow::query(const Conditions& qc)
{
    _logModel->query(qc);
    refreshTitle();
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
    model->addView(model);
    _logModel = model;
    _logView->setModel(model);
    _detailView->setModel(model);
    connect(model, SIGNAL(layoutChanged()), _logView, SLOT(dataChanged()));
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
    QAction *action;
    QWidgetAction *widgetAction;

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
    //QVariant v = _logModel->data(current, Qt::DisplayRole);
    this->setText(out);
    //this->setHtml(out);
}



