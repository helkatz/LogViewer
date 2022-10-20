#include <interfaces/logmodel.h>
#include <core/settings.h>
//#include <core/logupdater.h>

LogModel::LogModel(QObject *parent):
    QSqlQueryModel(parent),
    _rows(0),
    virtualRows_(true)
{
}

QVariant LogModel::data(int row, int col, int role) const
{
    QModelIndex index = createIndex(row, col);
    return data(index, role);
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
	if(role == Qt::TextAlignmentRole)
		return QFlag( Qt::AlignLeft | Qt::AlignTop);

    if(role != Qt::DisplayRole && role != Qt::ToolTipRole)
        return QVariant();

	// use currentRow for all other cols than the first one its faster then query in map
	if (index.column() > 0 && _currentRow.row == index.row())
		return _currentRow.record.value(index.column());

    DataCache::const_iterator it = _dataCache.find(index.row());
    if(it == _dataCache.end()) {
		loadData(index.row());
		if (_currentRow.row == index.row())
			return _currentRow.record.value(index.column());
    }

    if(it != _dataCache.end()) {
		_currentRow.set(index.row(), it->second);
        return _currentRow.record.value(index.column());
    }
    return "undef index";
}

void LogModel::writeSettings(_settings::LogWindow& s)
{
	s.queryParams()->set(qp_);
	//qp_->setPath("queryParams");
	//s->set(qp_);
	//qp_.saveCache();
    //qp_.writeSettings(s);
}

void LogModel::readSettings(_settings::LogWindow& s)
{
    // do not read queryParams because they are readed on startup
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    return _rows;
}

int LogModel::columnCount(const QModelIndex &parent) const
{
    return _columnsInformation.count();
}

QStringList LogModel::columns() const
{
    QStringList list;
    for(int i = 0; i < _columnsInformation.count(); i++)
        list.push_back(_columnsInformation.fieldName(i));
    return list;
}

QSqlRecord LogModel::columnsInformation() const
{
	return _columnsInformation;
}

quint64 LogModel::getFrontRow() const
{
	return 0;
}

quint64 LogModel::getBackRow() const
{
	return 0;
}

int LogModel::fetchMoreUpward(quint32, quint32)
{
	return 0;
}

int LogModel::fetchMoreDownward(quint32, quint32)
{
	return 0;
}

int LogModel::fetchToEnd()
{
	return 0;
}

int LogModel::fetchToBegin()
{
	return 0;
}

int LogModel::fetchMoreFromBegin(quint32 items)
{
	Q_UNUSED(items);
	return 0;

}

int LogModel::fetchMoreFromEnd(quint32 items)
{
	Q_UNUSED(items);
	return 0;

}

void LogModel::removeView(LogView *view)
{
	_views.removeOne(view);
	if (_views.empty()) {
		//ObserverBase::removeObserver(this);
	}
}

bool LogModel::virtualRows() const
{
    return virtualRows_;
}

const QueryParams& LogModel::getQueryParams() const
{
	return qp_;
}

void LogModel::setQueryParams(const QueryParams& qp)
{
	qp_ = qp;
}

//Conditions LogModel::getQueryConditions() const
//{
//	return qc_;
//}
//
//void LogModel::setQueryConditions(const Conditions & qc)
//{
//	qc_ = qc;
//}

void LogModel::followTail(bool enabled)
{
	//if (enabled)
	//	observer_.run();
	//else
	//	observer_.pause();
}

void LogModel::addView(LogView *view)
{
    _views.append(view);
}
//
//QueryParams::QueryParams(const QString& settingsPath):
//    settingsPath_(settingsPath)
//{
//    readSettings();
//}
//
//QString QueryParams::settingsPath() const
//{
//    return settingsPath_;
//}
//
//void QueryParams::writeSettings(_settings::QueryParams& s)
//{
//    //ApplicationSettings settings(settingsPath());
//    foreach(QString name, _hive->keys()) {
//        s.set(name, (*_hive)[name]);
//    }
//}
//
//void QueryParams::readSettings(_settings::QueryParams& s)
//{
//    foreach(QString name, s.childKeys()) {
//        (*_hive)[name] = s.get(name);
//        qDebug() << name << "=" << (*_hive)[name];
//    }
//}


