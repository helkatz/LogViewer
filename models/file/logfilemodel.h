#pragma once
#include <models/logmodel.h>
#include <models/file/logfile_parser.h>
//#include "Utils/utils.h"

//#include <QFile>
#include <QSharedPointer>
//#include <qfilesystemwatcher.h>
//#include <qthread.h>
//#include <qfile.h>
//#include <qtextstream.h>
//#include <qregularexpression.h>

class FileConditions: public Conditions
{
public:
    FileConditions():
        Conditions()
    {
    }
    FileConditions(const Conditions& other):
        Conditions(other)
    {
    }
    PROPERTY(FileConditions, QString, fileName)
};

class LogFileModel : public LogModel
{
	friend class LogFileModelTest;
    Q_OBJECT
    QSharedPointer<Parser> _parser;

protected:
    FileConditions qc() const
        { return _queryConditions; }

    //QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

	CurrentRow& loadData(uint64_t index) const override;

	quint64 getFrontRow() const override;

	quint64 getBackRow() const override;

	int fetchToEnd() override;

	int fetchToBegin() override;

	int fetchMoreBackward(quint32 row, quint32 items) override;

	int fetchMoreForward(quint32 row, quint32 items) override;

	int fetchMoreFromBegin(quint32 items) override;

	int fetchMoreFromEnd(quint32 items) override;

public:
    LogFileModel(QObject *parent);

	QString getTitle() const override;

	void writeSettings(const QString& basePath) override;

	void readSettings(const QString& basePath) override;

	bool query(const Conditions& QueryOptions) override;

	QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns,
		const QString& search, bool regex, bool down) const override;

    bool queryWithCondition(QString sqlFilter, int limit);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;


private slots:
    void observedObjectChanged(const QString& id, const int maxId);
    void entriesCountChanged(quint32 newCount);
};

