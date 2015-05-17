#ifndef LOGUPDATER_H
#define LOGUPDATER_H

#include <QThread>
#include <QSqlDatabase>
#include <QString>
#include <QTimer>
#include <QMap>
#include <vector>
class LogUpdater : public QThread
{
    Q_OBJECT
    struct ObserverData
    {
        QObjectList owners;
        QString connectionName;
        QString table;
        int fromPk;
        QString pk;
        QSqlDatabase db;
    };

private:
    QTimer _timer;
    QTimer _addDataTimer;
    QMap<QString, ObserverData *> _observerData;
    static LogUpdater *_instance;
    //QList<ObserverData> _observerDataList;

    explicit LogUpdater(QObject *parent = 0);
public:

    static LogUpdater& instance();


signals:
    void newDataReceived(const QString &connectionName, const QString& table, int maxId);

private slots:
    void run();
    void addData();
    void checkChanges();
public slots:
    void addTableObserver(const QString &connectionName, const QString& table);
    void removeTableObserver(QObject* object);
};

#endif // LOGUPDATER_H
