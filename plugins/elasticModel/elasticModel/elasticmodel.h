#pragma once
#include <core/settings.h>
#include <interfaces/logmodel.h>
#include <utils/utils.h>

using QueryParams = _settings::QueryParams;

SETTINGSCLASS(ElasticQueryParams, _settings::QueryParams,
	PROP(QString, host)
	PROP(QString, index)
);

class ElasticModelTest;
class ElasticModel : public LogModel
{
	Q_OBJECT
	friend class ElasticModelTest;


protected:

	CurrentRow& loadData(quint64 index) const override;

    //QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
public:
	ElasticModel(QObject *parent);

	QString getTitle() const override;

	void writeSettings(_settings::LogWindow&) override;

	void readSettings(_settings::LogWindow&) override;

	bool query() override;

	virtual QModelIndex find(const QModelIndex& fromIndex, const QStringList & columns,
		const QString& search, bool regex, bool down) const override;

    bool queryWithCondition(QString sqlFilter, int limit) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

	void processObserved() override;

private slots:
    void observedObjectChanged(const QString& id, const int maxId);
    void entriesCountChanged(quint32 newCount);

private:
	struct Private;
	QSharedPointer<Private> impl_;
};

