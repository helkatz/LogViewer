#include "utils.h"
#include <qsqlquery.h>

class SQLFilter
{
	QSqlDatabase _db;
	QSharedPointer<QSqlQuery> _q;
	QString _where;
	const QList<QString>& _names;
	const QList<QVariant>& _values;
public:
	SQLFilter(const QString& where, const QList<QString>& names, const QList<QVariant>& values) :
		_names(names),
		_values(values)
	{
		_db = utils::database::getDatabase("sqlite_internal", "QSQLITE");
		_db.setDatabaseName(":memory:");
		if (_db.open() == false)
			QMessageBox::information(0, "error", "could not open sqlite database");
		_q = QSharedPointer<QSqlQuery>(new QSqlQuery(_db));
		"Message like('dada') or level='debug'";
		QString from;
		foreach(QString name, names)
		{
			from += QString(",:%1 as `%1`").arg(name);
		}
		from = "select " + from.mid(1);
		QString sql = QString("select 1 as valid from (%2) where %1").arg(where).arg(from);
		if (_q->prepare(sql) == false) {
			qDebug() << "error:" << _q->lastError().text();
		}
	}
	bool execute()
	{
		int pos = 0;
		foreach(QString name, _names)
		{
			_q->bindValue(":" + name, _values.at(pos));
			pos++;
		}
		if (_q->exec() == false) {
			qDebug() << "error:" << _q->lastError().text();
		}
		return _q->first();
	}
};

QString& operator<<(QString &out, const QString& var)
{
	out += var;
	return out;
}
QString& operator<<(QString &out, const int var)
{
	out += QString("%1").arg(var);
	return out;
}

