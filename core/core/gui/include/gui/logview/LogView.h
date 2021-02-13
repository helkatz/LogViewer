#pragma once
#include <gui/logview/TextColorizer.h>
#include <gui/logview/SyntaxHighLighter.h>
#include <gui/findwidget.h>

#include <interfaces/LogModel.h>


#include <qwindow>
#include <QString>
#include <QDateTime>
#include <QWidget>
#include <QTableView>
#include <QSplitter>
#include <qstyleditemdelegate>
#include <QTextEdit>
#include <qplaintextedit.h>
#include <qtimer.h>
#include <boost/variant.hpp>

// #define USE_FROZENCOLUMNS
class LogModel;

class LogItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    LogItemDelegate(QObject *parent = 0);
    void paint(QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
    virtual QString xdisplayText(const QVariant & value, const QLocale & locale ) const;
protected:
    virtual void xdrawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QColor xgetColorFromString(QString s, bool sameRGB = false) const;
private slots:

private:
};

struct TColoredRowBackground
{
    int column; // the row background belongs to this column
	int colorizeByChars; // the color will calcualted
    int ligthness;
	TColoredRowBackground() { column = 0; colorizeByChars = 0; }
};


struct CellColorizer
{
private:
	struct Alternate
	{
		QString currentText;
		int currentColorIndex;
		Alternate() : currentColorIndex(0) {};
	};
	Alternate alternate;
public:
	int colorizeByChars;
	types::ColorList availableColors;
	bool alternateColors;
	CellColorizer():
		colorizeByChars(0),
		alternateColors(false)
	{ }

	QColor getColor(int column, const QString& text);
};

struct RowColorizer: public CellColorizer
{
	int boundColumn;
	RowColorizer():
		boundColumn(0)
	{ }
};

struct RowStyle
{
	TextColorizer textColorizer;
	QMap<int, QSharedPointer<CellColorizer>> cellColorizer;
	RowColorizer rowColorizer;
	bool colorizeFullRow;
	bool alternateRowColors;
	QFont font;
	int fontSize;
	void writeSettings(const QString& basePath);
	void readSettings(const QString& basePath);
	CellColorizer& getCellColorizer(int index)
	{
		if(cellColorizer.find(index) == cellColorizer.end())
			cellColorizer[index] = QSharedPointer<CellColorizer>(new CellColorizer());
		return *cellColorizer[index];
	}

	RowColorizer& getRowColorizer()
	{
		return rowColorizer;
	}
};

class LogView : public QTableView
{
    Q_OBJECT
	QString _settingsPath;
	RowStyle _rowStyle;
    FindWidget *_findWidget;
    bool _followMode;
	quint64 _lastVerticalScrollPos;

#ifdef USE_FROZENCOLUMNS
// freeze columns stuff
protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    //void scrollTo (const QModelIndex & index, ScrollHint hint = EnsureVisible);
private:
      QTableView *frozenTableView;
      void initFrozenTable();
      void updateFrozenTableGeometry();
private slots:
      void updateSectionWidth(int logicalIndex,int, int newSize);
      void updateSectionHeight(int logicalIndex, int, int newSize);
#endif
    virtual void keyPressEvent(QKeyEvent* e);
    bool eventFilter(QObject *target, QEvent *e);
public:
    LogView(QWidget *parent);
    ~LogView();
    LogModel *model() const { return dynamic_cast<LogModel *>(QTableView::model()); }
    bool followMode() const;
    void writeSettings(_settings::LogWindow&);
    void readSettings(_settings::LogWindow&);
    /*int getColorizeColumn(int index) const;
    TColoredRowBackground getColorizeRow() const;
    TextPartColorizer& getColorizer();
	ColorList getAvailableCellColors(int index) const;
	ColorList getAvailableRowColors() const { return _availableRowColors; }*/
	RowStyle& getRowStyle() { return _rowStyle; }
signals:
signals:
	void scrolltable(QModelIndex index);
	//void createLogWindow(const LogWindow::CreateParams& params);

protected slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void doubleClicked(const QModelIndex &);
    void testSlot(const QModelIndex &);
protected slots:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void showColumn(bool, int);
	void doScroll(QModelIndex index)
	{
		QTableView::scrollTo(index);
	};
#if 0
    void setColorizeColumn(int, int);
    void setColorizeRow(int colorize, int row);
    void setLigthnessRow(int ligthness, int row);
    void setTextPartColor(const QString& text, int color);
#endif
    void setFindTextColor(const QStringList& columns, const QString& text);
public slots:
    bool queryWithCondition(QString filter, int limit);
    void setFont(QFont font) { QTableView::setFont(font); }
    void setFontSize(int size) { QFont f = font(); f.setPointSize(size); setFont(f); }
    void setFollowMode(bool enabled = true);
    //void setAlternateRowColors(bool enabled = true);
    void contextMenu(const QPoint& point);
    int find(const QStringList& columns, const QString &search, bool regex = false, bool down = true);
    void dataChanged();
    void showFindWidget(bool show = true);
    void sliderMoved(int pos);
    void updateHeader();
	void setModifiedPos(quint32 row);
	void verticalScrollbarAction(int action);
	void verticalScrollbarValueChanged(int value);
	//void setAvailableCellColors(const ColorList& colors, int col);
	//void setAvailableRowColors(const ColorList& colors);

};

class DetailView: public QTextEdit
{
    Q_OBJECT
	QTextDocument _doc;
	QTimer _showTimer;
	LogModel *_logModel;
    QMap<int, bool> _visibleColumns;
	QStringList _lines;
	quint32 _lastVerticalScrollPos = -1;
	QModelIndex _currentIndex;
	QMetaObject::Connection _conn;
    Highlighter _highligter;
public:
    DetailView(QWidget *parent);
    void setModel(LogModel *model);
    LogModel *model() const { return _logModel; }
    void writeSettings(_settings::LogWindow&);
    void readSettings(_settings::LogWindow&);
public slots:
    void currentRowChanged(QModelIndex current, QModelIndex last);
    void setFont(QFont font) { QTextEdit::setFont(font); }
    void setFontSize(int size) { QFont f = font(); f.setPointSize(size); setFont(f); }
protected slots:
    void contextMenu(const QPoint &point);
    void showColumn(bool visible, int col);
};

class LogWindow: public QSplitter
{
	friend class LogWindowTest;
    Q_OBJECT
    QSharedPointer<LogModel> model_;
	QSharedPointer<LogView> mainView_;
	QSharedPointer<DetailView> detailView_;
protected:
    QString _settingsPath; // stored for creating subwindows
    LogWindow();
	~LogWindow();
public:
    //using CreateParam = boost::variant<_settings::LogWindow, _settings::QueryParams>;
    struct CreateParams {
        boost::optional<_settings::LogWindow> settings;
        boost::optional<_settings::QueryParams> queryParams;
        bool determineTemplateWindow = false;
        LogWindow* templateWindow = nullptr;
    };
    static LogWindow *create(CreateParams& params);
    //static LogWindow *create(QueryParams qc, LogView *templateView);
    //static LogWindow *create(const QString &settingsPath);

    QSharedPointer<LogView> getLogView() const;
    QSharedPointer<DetailView> getDetailView() const;

    bool queryWithCondition(QString, int);
    bool query();
    void setModel(QSharedPointer<LogModel> model);
	QSharedPointer<LogModel> model() const { return model_; }

    void refreshTitle();
    void writeSettings(_settings::LogWindow&);
    void readSettings(_settings::LogWindow&);
    void setSettings(LogWindow* from);

signals:
	void refreshWindowTitle();

public slots:
    void setFollowMode(bool enabled = true);
    void showFindWidget(bool show = true);

public:
	
};

