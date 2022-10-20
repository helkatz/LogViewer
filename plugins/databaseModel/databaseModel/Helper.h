#pragma once

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>

#include "Settings.h"
namespace helper {
    static QSqlDatabase cloneDatabase(const QSqlDatabase& other)
    {
        static int clone = 0;
        return QSqlDatabase::cloneDatabase(other, QString("__cloned__db%1").arg(++clone));
    }

    static QSqlDatabase getDatabase(const QString& name, const QString& driver, const QString& host, const QString& database, const QString& username, const QString& password)
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(driver, name);
        db.setDatabaseName(database);
        db.setHostName(host);
        db.setUserName(username);
        db.setPassword(password);
        if (db.open() == false) {
            QMessageBox::critical(NULL,
                QObject::tr("Unable to open database"),
                db.lastError().text());
            QSqlDatabase::removeDatabase(name);
            return QSqlDatabase();
        }
        return db;
    }

    static QSqlDatabase getDatabase(const QString& name, const QString& driver = "")
    {
        auto settings = appSettings()->as< DatabaseSettings>();
        QSqlDatabase db;
        if (QSqlDatabase::contains(name) == false) {
            auto s = settings.connections(name);
            db = QSqlDatabase::addDatabase(
                driver.length() ? driver : s.driver(),
                name);
            db.setDatabaseName(s.database());
            db.setHostName(s.host());
            db.setUserName(s.username());
            db.setPassword(s.password());
        }
        else
            db = QSqlDatabase::database(name);
        if (db.lastError().isValid())
            QMessageBox::critical(NULL,
                QObject::tr("Unable to open database"),
                db.lastError().text());
        return db;
    }
}