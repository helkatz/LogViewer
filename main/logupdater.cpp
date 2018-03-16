#include "logupdater.h"
#include "Utils/utils.h"

#define QT_NO_DEBUG_OUTPUT
#define QT_NO_DEBUG
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimerEvent>
#include <QSqlRecord>
#include <QSqlField>
#include <ostream>
LogUpdater *LogUpdater::_instance = nullptr;
QMap<QString, ObserverBase::Ptr> ObserverBase::_observers;
bool ObserverFile::processChanges()
{
	// when watched by system then do nothing
	if (watched)
		return false;
	if (_lastSize == _file.size())
		return false;
	handleFileChanged(_fileName);
	return true;
}

void ObserverFile::handleFileChanged(const QString& fileName)
{
	Q_UNUSED(fileName);
	emit observedObjectChanged(getId(), _lastSize);
	_lastSize = _file.size();
}

bool ObserverTable::processChanges()
{
	if (_db.open() == false)
		return false;
	utils::database::SqlQuery q(_db);
	QString sql = QString("select max(`%1`) as id from %2 where `%1`>%3")
		.arg(_pk).arg(_table).arg(_fromPk);
	log_debug() << sql;
	q.exec(sql);
	if (q.first()) {
		if (q.value(0).toInt() > _fromPk) {
			log_debug() << "changes detected in table " << _table;
			_fromPk = q.value(0).toInt();
			emit observedObjectChanged(getId(), _fromPk);
		}
	}
	q.finish();
	if (q.lastError().isValid()) {
		log_debug() << "error " << q.lastError().text();
	}
	return true;
}

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
	const int msgcount = 5;
	const char *messages[msgcount] = {
		"this is the first message",
		"a second short",
		"my name is helmut katz",
		"i like sailing in corfu",
		"and now a very very very long message to see whats hanppend when its exeeds the bounding area"
	};
#if 0
	log_debug() << "addData";

    QString insert;
    for(int i = 0; i <= rand()%1000; i++)
        insert += QString(",(null, now(), 0, '%1')").arg(messages[rand()%msgcount]);
    insert = insert.mid(1);
	static QSqlDatabase dbInsert = utils::database::cloneDatabase(utils::database::getDatabase("test"));
    if(dbInsert.open()) {
        dbInsert.exec("insert into log values" + insert);
    }
    if(dbInsert.lastError().isValid()) {
		log_debug() << "error " << dbInsert.lastError().text();
    }
#endif
	auto file = _fsopen("C:/logs/appandingtest.log", "a+", _SH_DENYNO);
	if (file) {
		static int count = 0;
		fputs(QString("2015-09-23 17:33:20.000|Logger|Level|testmessage %1\n")
			.arg(messages[rand() % msgcount]).toStdString().c_str(), file);
		fclose(file);
	}
	
}

void LogUpdater::checkChanges()
{
	ObserverBase::checkChanges();
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

#if 0

void LogUpdater::addTableObserver(QObject *object, const QString &connectionName, const QString& table)
{
	DEBUG() << "addTableObserver:" << object << " connection:" << connectionName << " table:" << table;
    QString mapName = ObserverTable::createId(connectionName, table);
	if (_observerData.find(mapName) != _observerData.end()) {
		_observerData[mapName]->owners.append(object);
		return;
	}
	ObserverTable::createObserver(object, connectionName, table);
	ObserverTable *od = new ObserverTable;

	od->owners.append(object);
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

void LogUpdater::addFileObserver(QObject *object, const QString& fileName)
{
	DEBUG() << "addFileObserver:" << object << " fileName:" << fileName;
	QString mapName = ObserverFile::createId(fileName);
	if (_observerData.find(mapName) != _observerData.end()) {
		_observerData[mapName]->owners.append(object);
		return;
	}
	ObserverFile::createObserver(object, fileName);
	ObserverFile *od = new ObserverFile;
	od->file.open(fileName);
	od->lastSize = od->file.size();
	od->fileName = fileName;
	od->owners.append(object);
	if (od->watcher.addPath(fileName) == false)
		QMessageBox::information(0, "error", "could not watch the logfile");
	QObject::connect(&od->watcher, SIGNAL(fileChanged(const QString&)), od, SLOT(handleFileChanged(const QString&)));
	QObject::connect(&LogUpdater::instance(), SIGNAL(newDataReceived(QString, int)), object, SLOT(dataChanged(QString, int)));
}


void LogUpdater::removeObserver(QObject *object)
{
    DEBUG()<<"removeTableObserver:"<<object;
    foreach(ObserverBase *od, _observerData) {
        QString mapName = od->getId();
        od->owners.removeOne(object);
        if(od->owners.empty()) {
            DEBUG()<<"no more owners remove map entry";
            _observerData.remove(mapName);
        }
    }
}
#endif

