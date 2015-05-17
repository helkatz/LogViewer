#ifndef LOGFILEMODEL_H
#define LOGFILEMODEL_H
#include "logmodel.h"
#include "queryconditions.h"
#include <QFile>
#include <QSharedPointer>
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
    PROPERTY(QString, fileName)
};
class Parser;
class LogFileModel : public LogModel
{
    Q_OBJECT
    QSqlQuery *_sqlQuery;
    QSharedPointer<QFile> _file;
    QSharedPointer<Parser> _parser;
    //FileQueryConditions _queryConditions;
protected:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    bool loadData(const QModelIndex &index) const;

    FileConditions qc() const
        { return _queryConditions; }
public:
    LogFileModel(QObject *parent);
    bool query(const Conditions& qqueryConditions);

    //FileQueryConditions& getQueryConditions()
      //  { return _queryConditions; }

    void writeSettings(const QString& basePath);

    void readSettings(const QString& basePath);

    QModelIndex find(const QModelIndex& fromIndex, QString where, bool down) const;

    QString getTitle() { return "LogFileModel"; }
};

#endif // LOGFILEMODEL_H
