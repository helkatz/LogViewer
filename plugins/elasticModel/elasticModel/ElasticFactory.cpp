#include "ElasticFactory.h"
#include <gui/logview/LogView.h>
#include <gui/querydialog.h>
#include <gui/OpenerDialog.h>
#include <core/common.h>

#include <QLineEdit>
#include <QCheckBox>

class ElasticOpenerDialog : public OpenerDialog
{
	QLineEdit *host;
	QLineEdit *user;
	QLineEdit *password;
	QLineEdit *index;
public:
	using OpenerDialog::OpenerDialog;
	ElasticOpenerDialog(QWidget *parent)
		: OpenerDialog(parent)
	{
		conne
		properties()->addRow("host", host = new QLineEdit(this));
		properties()->addRow("user", user = new QLineEdit(this));
		properties()->addRow("password", password = new QLineEdit(this));
		properties()->addRow("index", index = new QLineEdit(this));
	}
	ElasticConditions conditions()
	{
		ElasticConditions c;
		c.host(host->text());
		c.user(user->text());
		c.password(password->text());
		c.index(index->text());
	}
};

QString ElasticOpener::name() const
{
	return "ElasticOpener";
};

QWidget *ElasticOpener::createWidget(QWidget *parent)
{
	return new ElasticOpenerDialog(parent);
}

QList<Conditions> ElasticOpener::exec()
{
	QList<Conditions> ret;
	ElasticOpenerDialog d(nullptr);
	if (d.exec() == ElasticOpenerDialog::Accepted) {
		ElasticConditions c;
		c.host(d.host->text())
		ret.push_back(d.getQueryOptions());
	}
	return ret;
}

ElasticPlugin::ElasticPlugin()
{
	//registerType<ColumnizerWidget>("settings", "ColumnizerWidget");
	registerType<ElasticModel>("models", "ElasticModel");
	registerType<ElasticOpener>("openers", "ElasticOpener");
}
