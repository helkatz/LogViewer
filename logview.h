#ifndef LOGVIEW_H
#define LOGVIEW_H

#include "queryconditions.h"
#include "forms/findwidget.h"
#include "logsqlmodel.h"
#include <qwindow.h>
#include <QString>
#include <QDateTime>
#include <QWidget>
#include <QTableView>
#include <QtWebKitWidgets>
#include <QSplitter>

// #define USE_FROZENCOLUMNS

class LogItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    LogItemDelegate(QObject *parent = 0);
    void paint(QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const;
    virtual QString displayText(const QVariant & value, const QLocale & locale ) const;
protected:
    virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QColor getColorFromString(QString s, bool sameRGB = false) const;
private slots:

private:
};

struct TColoredRowBackground
{
    int column; // the row background belongs to this column
    int charsCount; // the color will calcualted
    int ligthness;
    TColoredRowBackground() { column = 0; charsCount = 0; }
};

struct TColoredTextPart
{
    QString textPart;
    QColor color;
};
typedef QMap<QString, TColoredTextPart>  TColoredTextParts;
class TextPartColorizer
{

    TColoredTextParts _coloredTextParts;
    //QMap<int, > _coloredTextParts;
public:
    void removeText(const QString& textPart);
    void addText(const QString& textPart, QColor color, int column = -1);
    void drawText(QPainter *painter, const QStyleOptionViewItem &option, int column, const QString& text);
    TColoredTextPart *getByText(const QString& text);
    const TColoredTextParts& getList() const { return _coloredTextParts; }

};

class LogView : public QTableView
{
    Q_OBJECT
    //QueryConditions *_queryConditions;
    //Q_PROPERTY(bool followMode READ followMode WRITE setFollowMode)
    FindWidget *_findWidget;
    bool _followMode;
    QMap<int, int> _coloredColumns;

    TColoredRowBackground _coloredRows;
    TextPartColorizer _textPartColorizer;
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
public:
    LogView(QWidget *parent);
    ~LogView();
    LogModel *model() const { return dynamic_cast<LogModel *>(QTableView::model()); }
    bool followMode() const;
    void writeSettings(const QString& basePath);
    void readSettings(const QString& basePath);
    int getColorizeColumn(int index) const;
    TColoredRowBackground getColorizeRow() const;
    TextPartColorizer& getColorizer();


signals:

protected slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void doubleClicked(const QModelIndex &);
    void testSlot(const QModelIndex &);
protected slots:
    virtual void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void showColumn(bool, int);
    void setColorizeColumn(int, int);
    void setColorizeRow(int colorize, int row);
    void setLigthnessRow(int ligthness, int row);
    void setTextPartColor(const QString& text, int color);

public slots:
    void queryWithCondition(QString filter, int limit);
    void setFont(QFont font) { QTableView::setFont(font); }
    void setFontSize(int size) { QFont f = font(); f.setPointSize(size); setFont(f); }
    void setFollowMode(bool enabled = true);
    void setAlternateRowColors(bool enabled = true);
    void contextMenu(const QPoint& point);
    int find(const QString &where, bool down = true);
    void dataChanged();
    void showFindWidget(bool show = true);
    void sliderMoved(int pos);
    void updateHeader();
};

inline int LogView::getColorizeColumn(int index) const
{
    return _coloredColumns.find(index) != _coloredColumns.end() ? _coloredColumns[index] : 0;
}

inline bool LogView::followMode() const
{
    return _followMode;
}

inline TColoredRowBackground LogView::getColorizeRow() const
{
    return _coloredRows;
}

inline TextPartColorizer& LogView::getColorizer()
{
    return _textPartColorizer;
}

class DetailView: public QTextEdit
{
    Q_OBJECT
    LogModel *_logModel;
    QMap<int, bool> _visibleColumns;
public:
    DetailView(QWidget *parent);
    void setModel(LogModel *model);
    LogModel *model() const { return _logModel; }
    void writeSettings(const QString &basePath);
    void readSettings(const QString &basePath);
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
    Q_OBJECT
    LogModel *_logModel;
    LogView *_logView;
    DetailView *_detailView;
protected:
    QString _settingsPath; // stored for creating subwindows
    LogWindow();
public:

    static LogWindow *create(Conditions qc, bool useTemplate = false);
    static LogWindow *create(const QString &settingsPath);

    void queryWithCondition(QString, int);
    void query(const Conditions &qc);
    void setModel(LogModel *model);
    LogModel *model() const { return _logModel; }

    void refreshTitle();
    void writeSettings(const QString& basePath);
    void readSettings(const QString& basePath);
public slots:
    void setFollowMode(bool enabled = true);
    void showFindWidget(bool show = true);
};

#endif // LOGVIEW_H
