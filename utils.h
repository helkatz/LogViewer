#ifndef UTILS_H
#define UTILS_H
#include "settings.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QTranslator>
class MySqlDatabase: public QSqlDatabase
{
public:
    MySqlDatabase(QString type):
      QSqlDatabase(type)
    {

    }
};

class Utils
{
public:
    Utils();
    static QSqlDatabase cloneDatabase(const QSqlDatabase& other)
    {
        static int clone = 0;
        return QSqlDatabase::cloneDatabase(other, QString("__cloned__db%1").arg(++clone));
        MySqlDatabase db(other.driverName());
        db.setDatabaseName(other.databaseName());
        db.setHostName(other.hostName());
        db.setPassword(other.password());
        db.setUserName(other.userName());
        db.setDatabaseName(other.databaseName());
        //QSqlDatabase::removeDatabase("__cloned__db");
        return db;
    }

    static QSqlDatabase getDatabase(const QString& name, const QString& driver, const QString& host, const QString& database, const QString& username, const QString& password)
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(driver, name);
        db.setDatabaseName(database);
        db.setHostName(host);
        db.setUserName(username);
        db.setPassword(password);
        if(db.open() == false) {
            QMessageBox::critical(NULL,
                QObject::tr("Unable to open database"),
                db.lastError().text());
            QSqlDatabase::removeDatabase(name);
            return QSqlDatabase();
        }
        return db;
    }

    static QSqlDatabase getDatabase(const QString& name)
    {
        QSqlDatabase db;
        if(QSqlDatabase::contains(name) == false) {
            Settings settings;
            db = QSqlDatabase::addDatabase(
                    settings.connections(name).driver(),
                    name);
            db.setDatabaseName(settings.connections(name).database());
            db.setHostName(settings.connections(name).host());
            db.setUserName(settings.connections(name).username());
            db.setPassword(settings.connections(name).password());
        } else
            db = QSqlDatabase::database(name);
        if(db.lastError().isValid())
            QMessageBox::critical(NULL,
                QObject::tr("Unable to open database"),
                db.lastError().text());
        return db;
    }
};


QString& operator<<(QString &out, const QString& var);
QString& operator<<(QString &out, const int var);
#endif // UTILS_H
