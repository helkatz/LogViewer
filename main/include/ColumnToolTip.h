#pragma once
#include <QObject>
#include <QAbstractItemView>
#include <QHelpEvent>
#include <QToolTip>

class ColumnToolTip : public QObject
{
	Q_OBJECT
public:
	explicit ColumnToolTip(QObject* parent = NULL);

protected:
	bool eventFilter(QObject* obj, QEvent* event);
};

