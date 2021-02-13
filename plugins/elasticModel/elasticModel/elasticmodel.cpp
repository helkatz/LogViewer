#include "elasticmodel.h"
#include "private.h"

#include <core/logupdater.h>
#include <core/common.h>
#include <utils/utils.h>

#include <QFile>
#include <QTextStream>
#include <QRegularExpressionMatch>
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QUrlQuery>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>
#include <QThread>
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qjsonarray.h>
#include <string>
#include <deque>
#include <iostream>
#include <Poco/JSON/JSON.h>
#include <boost/optional.hpp>

#include <qdatastream.h>
#include <mutex>
#include <WinSock2.h>

#include <iostream>
#include <istream>
#include <ostream>
#include <string>

#include <fstream>

ElasticModel::ElasticModel(QObject *parent)
	: LogModel(parent)
	, impl_(new Private(this))
{
}

LogModel::CurrentRow& ElasticModel::loadData(quint64 index) const
{	
	log_func_entry_leave();
	log_debug() << "rowIndex" << index << "not found in dataCache{"<<_dataCache.size()<<"}";
__LTryAgain:
	try {
		request::QueryRange range = impl_->getQueryRangeFromIndex(index);

		request::Documents::Criteria criteria{ range, impl_->conditions_.queryString() };
		request::Documents::Response resp;

		impl_->request_.fetch<request::Documents>(criteria, resp);
		
		uint32_t cacheIndex = range.index;
		_currentRow.reset();
		if (resp.hits.docs.size()) {
			for (auto& doc : resp.hits.docs) {
				QSqlRecord r = _columnsInformation;
				for (auto& field : doc.source) {
					auto pos = r.indexOf(field.first.c_str());
					QSqlField f;
					f.setName(field.first.c_str());
					f.setValue(field.second.c_str());
					r.replace(pos, f);
				}
				if (cacheIndex == index) {
					_currentRow.set(index, r);
				}
				_dataCache[cacheIndex++] = r;
			}
		}
		return _currentRow;
	}
	catch (...) {
		log_debug() << "failed to load";
		QThread::sleep(1);
		goto __LTryAgain;
	}
}

bool ElasticModel::query(const Conditions &queryConditions)
{	
	log_func_entry_leave();

	_dataCache.clear();
	_columnsInformation.clear();

	uint32_t col = 0;
	impl_->request_.host = impl_->conditions_.host();
	impl_->request_.index = impl_->conditions_.index();
	impl_->request_.type = impl_->conditions_.type();
	request::Mappings::Response mappings;
	request::QueryConditions criteria;
	impl_->request_.fetch<request::Mappings>(criteria, mappings);

	foreach(const auto& name, mappings.properties) {
		QSqlField f;
		f.setName(name.c_str());
		_columnsInformation.append(f);
		setHeaderData(col++, Qt::Horizontal, tr("%1").arg(capitalize(name.c_str())));
	}

	impl_->loadQueryRangeList(queryConditions);

	observer_.install(std::chrono::milliseconds{ 1000 }, [this] {
		if(loadData(_rows))
			emit layoutChanged();
	});
	emit layoutChanged();
    return true;
}

bool ElasticModel::queryWithCondition(QString sqlFilter, int limit)
{
    impl_->conditions_.queryString(sqlFilter);
    impl_->conditions_.limit(limit);
    return query(impl_->conditions_);
}

void ElasticModel::writeSettings(const QString &basePath)
{
    impl_->conditions_.writeSettings(basePath);
}

void ElasticModel::readSettings(const QString &basePath)
{
    impl_->conditions_.readSettings(basePath);
}

QString ElasticModel::getTitle() const
{
    return impl_->conditions_.host();
}

void ElasticModel::observedObjectChanged(const QString& id, const int maxId)
{
}

void ElasticModel::entriesCountChanged(quint32 newCount)
{
}

QModelIndex ElasticModel::find(const QModelIndex& fromIndex, const QStringList & columns, const QString& search, bool regex, bool down) const
{
	Q_UNUSED(regex);
	Q_UNUSED(columns);
    qDebug() << "find fromPos:" << fromIndex.row() << "dir:" << down << "where:" << search;
    QModelIndex index = fromIndex;
	return index;
}

QModelIndex ElasticModel::index(int row, int column, const QModelIndex &parent) const
{
    auto index = QSqlQueryModel::index(row, column, parent);
    if (index.isValid())
        return index;
    if (row >= rowCount()) {
        //auto delta = row - rowCount();
        //const_cast<ElasticModel *>(this)->updateRowCount(row);
        //emit const_cast<ElasticModel *>(this)->layoutChanged();
    }
    return QSqlQueryModel::index(rowCount() - 1, column, parent);

}

void ElasticModel::processObserved()
{
	loadData(_rows);
}

