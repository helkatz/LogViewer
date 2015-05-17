#include "logupdater.h"
#include "utils.h"

#define QT_NO_DEBUG_OUTPUT
#define QT_NO_DEBUG
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimerEvent>
#include <QSqlRecord>
#include <QSqlField>
#include <ostream>
//#define DEBUG() qDebug()
#define DEBUG() if(0) qDebug()
LogUpdater *LogUpdater::_instance = NULL;

LogUpdater::LogUpdater(QObject *parent) :
    QThread(parent)
{
    moveToThread(this) ;
    start();
}

LogUpdater& LogUpdater::instance()
{
    if(_instance == NULL) {
        _instance = new LogUpdater(NULL);
    }
    return *_instance;
}

void LogUpdater::addData()
{
    qDebug()<<"addData";
    const int msgcount = 5;
    const char *messages[msgcount] = {
        "this is the first message",
        "a second short",
        "my name is helmut katz",
        "i like sailing in corfu",
        "and now a very very very long message to see whats hanppend when its exeeds the bounding area"
    };
    QString insert;
    for(int i = 0; i <= rand()%1000; i++)
        insert += QString(",(null, now(), 0, '%1')").arg(messages[rand()%msgcount]);
    insert = insert.mid(1);
    static QSqlDatabase dbInsert = Utils::cloneDatabase(Utils::getDatabase("test"));
    if(dbInsert.open()) {
        dbInsert.exec("insert into log values" + insert);
    }
    if(dbInsert.lastError().isValid()) {
        qDebug()<<"error "<<dbInsert.lastError().text();
    }
}

void LogUpdater::checkChanges()
{
    DEBUG()<<"checkChanges";
    foreach(ObserverData *od, _observerData) {
        if(od->db.open() == false)
            continue;
        QSqlQuery q(od->db);
        QString sql = QString("select max(`%1`) as id from %2 where `%1`>%3")
                .arg(od->pk).arg(od->table).arg(od->fromPk);
        DEBUG()<<sql;
        q.exec(sql);
        if(q.first()) {
            if(q.value(0).toInt() > od->fromPk) {
                DEBUG()<<"changes detected in table "<<od->table;
                od->fromPk = q.value(0).toInt();
                emit newDataReceived(od->connectionName, od->table, od->fromPk);
            }
        }
        q.finish();
        if(q.lastError().isValid()) {
            DEBUG()<<"error "<<q.lastError().text();
        }
    }
}

void LogUpdater::run()
{
    QTimer addDataTimer;
    QTimer checkChangesTimer;
    connect(&addDataTimer, SIGNAL(timeout()), this, SLOT(addData()));
    connect(&checkChangesTimer, SIGNAL(timeout()), this, SLOT(checkChanges()));
    //addDataTimer.start(5000);
    checkChangesTimer.start(1000);
    exec();

}

void LogUpdater::addTableObserver(const QString &connectionName, const QString& table)
{
    DEBUG()<<"addTableObserver:"<<sender()<<" connection:"<<connectionName<<" table:"<<table;
    QString mapName = connectionName + "_" + table;
    if(_observerData.find(mapName) != _observerData.end()) {
        ObserverData *od = _observerData[mapName];
        od->owners.append(sender());
        return;
    }
    ObserverData *od = new ObserverData;

    od->owners.append(sender());
    od->connectionName = connectionName;
    od->table = table;
    od->fromPk = 0;
    od->db = Utils::getDatabase(connectionName);

    od->db = Utils::cloneDatabase(od->db);
    od->db.open();
    QSqlRecord r = od->db.record(table);
    for(int col = 0; col < r.count(); col++) {
        QSqlField& f = r.field(col);
        if(f.isAutoValue()) {
            od->pk = r.fieldName(col);
            break;
        }
    }
    DEBUG()<<"add observer with pk:"<<od->pk;
    if(od->pk.length() > 0)
        _observerData[mapName] = od;
}

void LogUpdater::removeTableObserver(QObject *object)
{
    DEBUG()<<"removeTableObserver:"<<object;
    foreach(ObserverData *od, _observerData) {
        QString mapName = od->connectionName + "_" + od->table;
        od->owners.removeOne(object);
        if(od->owners.empty()) {
            DEBUG()<<"no more owners remove map entry";
            _observerData.remove(mapName);
        }
    }
}

#if 0
#include <QtCore>

class Thread : public QThread
{
private:
    void run()
    {
        DEBUG()<<"From worker thread: "<<currentThreadId();
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    DEBUG()<<"From main thread: "<<QThread::currentThreadId();

    Thread t;
    QObject::connect(&t, SIGNAL(finished()), &a, SLOT(quit()));

    t.start();
    return a.exec();
}
#endif
