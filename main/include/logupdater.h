#ifndef LOGUPDATER_H
#define LOGUPDATER_H

#include <QThread>
#include <QSqlDatabase>
#include <qsqlrecord.h>
#include <qsqlfield.h>
#include <QString>
#include <QTimer>
#include <QMap>
#include <vector>
#include <qfilesystemwatcher.h>
#include "Utils/utils.h"
#include <common/File.h>
class OnChangeHandler : public QObject
{
	Q_OBJECT;
public:
	using OnObjectChanged = std::function<void(const QString&, int)>;
	OnObjectChanged _onObjectChanged;

private slots:
	void observedObjectChanged(const QString &id, int maxId)
	{
		_onObjectChanged(id, maxId);
	}
};
class ObserverBase : public QObject
{
	Q_OBJECT;

	typedef QSharedPointer<ObserverBase> Ptr;

	static QMap<QString, ObserverBase::Ptr> _observers;
	
	
	QList<const QObject *> _owners;

protected:
	//using OnObjectChanged = std::function<void(const QString&, int)>;
	//OnObjectChanged _onObjectChanged;
	OnChangeHandler _onChangeHandler;
	virtual bool processChanges() = 0;

	virtual QString getId() const = 0;

	static bool has(const QString& id)
	{
		return _observers.find(id) != _observers.end();
	}

	static bool append(const QString& id, const ObserverBase::Ptr& data)
	{
		_observers[id] = data;
		return true;
	}

	static bool append(const QString& id, const QObject *owner)
	{
		if (has(id) == false)
			return false;
		QObject::connect(_observers[id].data(), SIGNAL(observedObjectChanged(QString, int)), owner, SLOT(observedObjectChanged(QString, int)));
		return true;
	}

	/*
	static bool appendWhenExists(const QObject *owner, const QString& id)
	{
		if (_observers.find(id) != _observers.end()) {
			_observers[id]->_owners.append(owner);
			return true;
		}
		return false;
	}*/
public:
	static void removeObserver(QObject *owner)
	{
		qDebug() << "removeTableObserver:" << owner;
		foreach(ObserverBase::Ptr od, _observers)
		{
			QString mapName = od->getId();
			od->_owners.removeOne(owner);
			if (od->_owners.empty()) {
				qDebug() << "no more owners remove map entry";
				_observers.remove(mapName);
			}
		}
	}

	static void pauseObserver(QObject *owner)
	{
		foreach(ObserverBase::Ptr od, _observers)
		{
			QString mapName = od->getId();
			od->_owners.removeOne(owner);
			if (od->_owners.empty()) {
				qDebug() << "no more owners remove map entry";
				_observers.remove(mapName);
			}
		}

	}

	static void checkChanges()
	{
		//qDebug() << "checkChanges";
		foreach(ObserverBase::Ptr od, _observers)
		{
			od->processChanges();
		}
	}
signals:
	void observedObjectChanged(const QString &id, int maxId);

};

class ObserverFile : public ObserverBase
{
	Q_OBJECT
protected:
	quint64 _lastSize;
	common::File _file;
	QString _fileName;
	QFileSystemWatcher _watcher;
	bool watched;

	bool processChanges();

	QString getId() const
	{
		return createId(_fileName);
	}

public:

	static QString createId(const QString& fileName)
	{
		return "file-id:" + fileName;
	}

	static bool createObserver(const QObject *owner, const QString& fileName
		, OnChangeHandler::OnObjectChanged onChanged = nullptr)
	{


		QSharedPointer<ObserverFile> od = QSharedPointer<ObserverFile>(new ObserverFile);
		od->_file.open(fileName.toStdString());
		od->_lastSize = od->_file.size();
		od->_fileName = fileName;
		//od->_owners.append(owner);
		od->watched = od->_watcher.addPath(fileName + "x");
		od->_onChangeHandler._onObjectChanged = onChanged;

		const QObject *own = owner;
		if (onChanged)
			own = &od->_onChangeHandler;
		QString mapName = createId(fileName);
		if (append(mapName, own))
			return true;

		QObject::connect(&od->_watcher, SIGNAL(fileChanged(const QString&)), od.data(), SLOT(handleFileChanged(const QString&)));
		//QObject::connect(od.data(), SIGNAL(observedObjectChanged(QString, int)), owner, SLOT(observedObjectChanged(QString, int)));
		//_observers[mapName] = od;
		append(mapName, od);
		append(mapName, own);
		
		return true;
	}
private slots:
	void handleFileChanged(const QString&);
};

class ObserverTable : public ObserverBase
{
	Q_OBJECT
protected:
	QString _connectionName;
	QString _table;
	int _fromPk;
	QString _pk;
	QSqlDatabase _db;
	
	bool processChanges();
	QString getId() const
	{
		return createId(_connectionName, _table);
	}

public:
	static QString createId(const QString& connectionName, const QString& table)
	{
		return "db-id:" + connectionName + "_" + table;
	}

	static bool createObserver(const QObject *owner, const QString& connectionName, const QString& table)
	{		
		QString mapName = createId(connectionName, table);
		if (append(mapName, owner))
			return true;

		QSharedPointer<ObserverTable> od = QSharedPointer<ObserverTable>(new ObserverTable);
		od->_connectionName = connectionName;
		od->_table = table;
		od->_fromPk = 0;
		od->_db = utils::database::getDatabase(connectionName);

		od->_db = utils::database::cloneDatabase(od->_db);
		od->_db.open();
		QSqlRecord r = od->_db.record(table);
		for (int col = 0; col < r.count(); col++) {
			QSqlField& f = r.field(col);
			if (f.isAutoValue()) {
				od->_pk = r.fieldName(col);
				break;
			}
		}
		/*qDebug() << "add observer with pk:" << od->_pk;
		if (od->_pk.length() > 0)
			_observers[mapName] = od;*/
		append(mapName, od);
		append(mapName, owner);
		//connect(od.data(), SIGNAL(observedObjectChanged(QString, int)), owner, SLOT(observedObjectChanged(QString, int)));
		return true;
	}
};

class LogUpdater : public QThread
{
    Q_OBJECT

private:
    QTimer _timer;
    QTimer _addDataTimer;
    static LogUpdater *_instance;
	
    explicit LogUpdater(QObject *parent = 0);
public:

    static LogUpdater& instance();


signals:

private slots:
    void run();
    void addData();
    void checkChanges();
public slots:
#if 0
	void addTableObserver(QObject *object, const QString &connectionName, const QString& table);
	void addFileObserver(QObject *object, const QString &fileName);
	void removeObserver(QObject* object);
#endif
};

#endif // LOGUPDATER_H
