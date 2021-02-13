#pragma once
#if defined(_WINDOWS) && defined(fileModel_EXPORTS)
#define FILEMODEL_API __declspec(dllexport)
#else
#define FILEMODEL_API __declspec(dllimport)
#endif
#include <interfaces/logmodel.h>
#include <gui/MainWindow.h>

#include <QSharedPointer>
#include <QAction>
#include <QApplication>
#include <QMenuBar>

SETTINGSCLASS(FileQueryParams, QueryParams,
	PROP(QString, fileName)
	PROP(QString, columnizer)
);

class FILEMODEL_API LogFileModel : public LogModel
{
	struct Private;
	QSharedPointer<Private> impl_;

    Q_OBJECT
protected:
	CurrentRow& loadData(quint64 index) const override;

	quint64 getFrontRow() const override;

	quint64 getBackRow() const override;

	int fetchToEnd() override;

	int fetchToBegin() override;

	int fetchMoreUpward(quint32 row, quint32 items) override;

	int fetchMoreDownward(quint32 row, quint32 items) override;

	int fetchMoreFromBegin(quint32 items) override;

	int fetchMoreFromEnd(quint32 items) override;

public:
    LogFileModel(QObject *parent);

	QString getTitle() const override;

	bool query() override;

	QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns,
		const QString& search, bool regex, bool down) const override;

    bool queryWithCondition(QString sqlFilter, int limit);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;


private slots:
    void observedObjectChanged(const QString& id, const int maxId);
    void entriesCountChanged(quint32 newCount);
};
