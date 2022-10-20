#include "ElasticFactory.h"
#include <gui/logview/LogView.h>
#include <gui/OpenerDialog.h>
#include <core/common.h>

#include <QLineEdit>
#include <QCheckBox>

QString ElasticOpener::name() const
{
	return "ElasticOpener";
};

QWidget *ElasticOpener::createWidget(QWidget *parent)
{
	return new QDialog(parent);
}

QList<QueryParams> ElasticOpener::exec()
{
	QList<QueryParams> ret;
	ElasticQueryParams qp;
	qp.index("logstash_syslog_ilm");
	qp.modelClass("ElasticModel");
	qp.host("http://test-elk-elasticsearch.test.srvint.ix2");
	ret.push_back(qp);
	return ret;
}

ElasticPlugin::ElasticPlugin()
{
	//registerType<ColumnizerWidget>("settings", "ColumnizerWidget");
	registerType<ElasticModel>("models", "ElasticModel");
	registerType<ElasticOpener>("openers", "ElasticOpener");
}
