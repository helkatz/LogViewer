#pragma once
#include <core/settings.h>
#include <interfaces/logmodel.h>
#include <utils/utils.h>

class ElasticConditions: public Conditions
{
public:	
	ElasticConditions()
	{}

	ElasticConditions(const Conditions& other):
		Conditions(other)
	{}

	PROPERTY(ElasticConditions, QString, connection, "");
	PROPERTY(ElasticConditions, QString, index, "");
	PROPERTY(ElasticConditions, QString, type, "");
	PROPERTY(ElasticConditions, QString, user, "");
	PROPERTY(ElasticConditions, QString, password, "");
};


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

	virtual QString getTitle() const override;

	void writeSettings(const QString& basePath) override;

	void readSettings(const QString& basePath) override;

	bool query(const Conditions& QueryOptions) override;

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

