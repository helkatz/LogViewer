#include <gui/settings_dialog.h>
#include <core/types.h>
#include <core/settings_new.h>
#include <Windows.h>
#include <Winbase.h>
#include <gtest/gtest.h>
#include <QApplication>
#include <QRegularExpression>

template<typename T>
class _PropList : public T, public QMap<QString, T>
{
	friend T;
	QMap<QString, T> list_;
public:
	//using T::T;
	_PropList(const QString& name, PropClass::PrivatePtr parent)
		: T(name, parent) 
	{
		for (auto group : childGroups()) {
			this->insert(name, T(QString(name) + "/" + group, impl_));
		}
	}
};

SETTINGSCLASS(Address, PropClass,
	PROP(int, plz)
	PROP(QString, street)
);

SETTINGSCLASS(Order, PropClass,
	_CONFIGURE(
		setPath("bindtest");
	)
	PROP(QString, articel_id)
	PROP(QString, articel)
);

SETTINGSCLASS(Person, PropClass,
	PROP(int, age)
	PROP(QString, name)
	_PROPCLASS(Address, address)
	_PROPLIST(Order, orders)
);

SETTINGSCLASS(TestSettings, PropClass,
	_CONFIGURE(
		setPath("");
		configurePersistent("ACOM", "LogViewer_Test");
	)
	_PROPLIST(Person, persons);
	//_PROPCLASS(_PropList<Person>, persons2);

);
TEST(Settings, path)
{
	TestSettings s;
	ASSERT_EQ("persons", s.persons()->path());
	ASSERT_EQ("persons/1", s.persons(1)->path());
	ASSERT_EQ("persons/1/address", s.persons(1).address()->path());

	auto address = s.persons(1).address();
	ASSERT_EQ("persons/1/address", address->path());

	address->unbind();
	ASSERT_EQ("address", address->path());
	s.persons(2)->bind(address, true);

	ASSERT_EQ("persons/2/address", address->path());
}

TEST(Settings, path2)
{
	TestSettings s;
	auto person = s.persons(1);
	ASSERT_EQ("persons", s.persons()->path());
	ASSERT_EQ("persons/1", s.persons(1)->path());
	ASSERT_EQ("persons/1/address", s.persons(1).address()->path());

	auto address = s.persons(1).address();
	ASSERT_EQ("persons/1/address", address->path());

	address->unbind();
	ASSERT_EQ("address", address->path());
	s.persons(2)->bind(address, true);

	ASSERT_EQ("persons/2/address", address->path());
}

TEST(Settings, get)
{
	const QString street = "teststreet";
	const QString street2 = "teststreet2";
	TestSettings s;
	s.persons(1).age(20);
	ASSERT_EQ(20, s.persons(1).age());
	auto address = s.persons(1).address();
	address.street(street);
	ASSERT_EQ(street, s.persons(1).address().street());
	ASSERT_EQ(street, address.street());
	address->unbind();
	address.street(street2);
	ASSERT_EQ(street, s.persons(1).address().street());
	ASSERT_EQ(street2, address.street());
	s.persons(1)->bind(address, true);
	ASSERT_EQ(street2, s.persons(1).address().street());
}

TEST(Settings, change_name)
{
	TestSettings s;
	auto before = s.personsList();
	s.persons("philip").age(20);
	s.persons("philip")->rename("philip2");
	auto after = s.personsList();
	ASSERT_EQ(1, s.personsList().size());
	auto l = s.personsList();

}
#if 0
TEST(Settings, list_new)
{
	ASSERT_EQ("persons2", s.persons2().path());
	Person person;
	person.age(18);
	person.name("philip");
	s.persons2().insert("1", person);

	ASSERT_EQ(1, s.persons2().size());
	//s.persons2()
	//_PropList<Person> persons2("persons2");
	//for (auto person : s.persons2()) {

	//}

}
#endif
#if 0
TEST(Settings, local_storage)
{
	TestProps s;
	ASSERT_EQ(QVariant::Invalid, s.intValue());
	s.intValue(10);
	s.inner().intValue(20);
	ASSERT_EQ(10, s.intValue());
	ASSERT_EQ(20, s.inner().intValue());
	s.inners(1).intValue(1);
	s.inners(2).intValue(2);
	s.inners(1).remove();
	auto list = s.innersList();
	list["1"].intValue(10);


}

TEST(Settings, persistent_storage)
{
	{
		auto s = TestSettings().propsClass();
		s.intValue(10);
		s.inner().intValue(20);
		ASSERT_EQ(10, s.intValue());
		ASSERT_EQ(20, s.inner().intValue());
	}
	{
		auto s = TestSettings().propsClass();
		ASSERT_EQ(10, s.intValue());
		ASSERT_EQ(20, s.inner().intValue());
	}
	{
		auto s = TestSettings().propsClass();
		// bind inner to /props so fullpath would be /props/bindtest/...
		BindTestProps inner(s);
		ASSERT_EQ(20, inner.intValue());
	}
	{
		TestSettings s;
		BindTestProps inner(s);
		ASSERT_EQ(20, inner.intValue());
	}
}

TEST(Settings, keys)
{
	{
		TestSettings s;
		s.intValue(5);
		s.stringValue("string");
		s.propsList(1).intValue(5);
		s.propsList(2).intValue(5);
		s.propsList(3).intValue(5);
		s.childKeys(PropClass::FetchKeysMode::NodeKeys);
		s.childKeys(PropClass::FetchKeysMode::NodeGroups);
		s.childKeys(PropClass::FetchKeysMode::All);
		ASSERT_EQ(3, s.propsList().childKeys(PropClass::FetchKeysMode::NodeGroups).size());
	}
}

TEST(Settings, bindTo)
{
	TestSettings s;
	auto inner = s.propsList(1).inner();
	inner.intValue(1);
	inner.stringValue("s1");
	ASSERT_EQ(1, inner.intValue());
	s.propsList(1).inner().intValue(2);
	ASSERT_EQ(2, inner.intValue());
	inner.unbind();

	s.propsList(1).inner().intValue(3);
	// still 2 becasue its unbounded
	ASSERT_EQ(2, inner.intValue());
	inner.intValue(1);
	ASSERT_EQ(1, inner.intValue());
	ASSERT_EQ(3, s.propsList(1).inner().intValue());
	inner.bindTo(s.propsList(2));
	ASSERT_EQ(2, s.propsList(2).inner().intValue());
}
TEST(Settings, childGroups)
{
	{
		auto s = TestSettings().propsClass();
		s.intValue(10);
		s.inner().intValue(20);
		ASSERT_EQ(10, s.intValue());
		ASSERT_EQ(20, s.inner().intValue());
	}
	{
		auto s = TestSettings().propsClass();
		ASSERT_EQ(10, s.intValue());
		ASSERT_EQ(20, s.inner().intValue());
	}
}
TEST(Settings, different_storage)
{
	Settings::setOrganisation("ACOM");
	Settings::setApplication("LogViewer_Test");
	TestSettings s;
	//s.intValue(10);
	ASSERT_EQ(10, s.intValue());
	TestProps props;
	props.intValue();
	
}
//class Struct
TEST(Settings, all)
{
	Settings::setOrganisation("ACOM");
	Settings::setApplication("LogViewer_Test");
	settings_new::ApplicationSettings as;
	as.windows(1);
	settings_new::QueryParams qp;
	qDebug() << qp.toTime();
	for (auto& window : as.windowsList()) {
		
	}
	//qDebug() << ts.qpList(1).childGroups();
	//auto qp = ts.qpList(1);
	//qp.params();
	//qDebug() << qp.path();
	/*
	Settings::setOrganisation("ACOM");
	Settings::setApplication("LogViewer");

	_settings::ApplicationSettings settings("/");
	qDebug() << settings.column("0").visible();
	settings.column("0").visible(1);
	qDebug() << settings.column("0").visible();
	qDebug() << settings.column().childGroups();
	qDebug() << settings.general().logFile();
	settings.view().rowStyle().textColorizer("0").caseSensitive(true);
	auto s = settings.view().rowStyle().textColorizer(0);
	qDebug() << s.text();
	s.text("text1");
	qDebug() << settings.view().rowStyle().textColorizer().childGroups();
	qDebug() << settings.view().rowStyle().textColorizer(0).text();
	{
		LogFileSettings lfs;
		qDebug() << lfs.columnizer().childGroups();
		auto s = lfs.columnizer("fdfsf");
		s.columns(0).fmtFunc("func");
		s.columns(5).fmtFunc("func1");
		s.columns(5).enabled(true);
		qDebug() << s.columns(0).fmtFunc();
		qDebug() << s.columns().childGroups();
		lfs.columnizer().dump();
		lfs.columnizer("ruleman").dump();
		lfs.columnizer("ruleman").columns().dump();
	}
	types::ColorList list;
	list.push_back(QColor());
	QSettings qs("ACOM", "LogViewer");
	*/

}
TEST(SettingsDialog, action)
{
	types::ColorList list;
	QVariant v;
	v.setValue(list);
	SettingsDialog dlg;
	dlg.exec();
}
#endif
#if 0
using namespace logger;

class GuiPlay
{
	void playRowStyleWidget
	{
		RowStyle rs;
		QTableWidget tv;
		tv.setRowCount(3);
		tv.setColumnCount(4);
		RowLayoutWidget rlw(nullptr, rs, tv.model()->index(0, 0));
		rlw.show();
		a.exec();
	}
};
void WidgetTest::ConnectionsWidgetTest()
{
	ConnectionsWidget w;
	w.resize(500, w.height());
	w.exec();
	exit(0);
}
void WidgetTest::RowLayoutWidgetTest()
{
/*	RowStyle style;
	RowLayoutWidget w(nullptr, style, QModelIndex());
	w.resize(1000, w.height());
	w.exec();
	exit(0);*/
}
void WidgetTest::ColumnizerWidgetTest()
{
	ColumnizerWidget w;
	w.resize(1000, w.height());
	w.exec();
	exit(0);
}


void LogWindowTest::run()
{
	while (!MainWindow::_instance)
		Sleep(500);
	auto& area = *MainWindow::instance().mdiArea;
	while (area.subWindowList().size() == 0)
		Sleep(1000);
	QMdiSubWindow *window = area.subWindowList().at(0);
		
	//area.setActiveSubWindow(window);
	LogWindow *logView = qobject_cast<LogWindow *>(window->widget());
	QTimer scrollTimer;
//	log_trace(0) << "thread" << currentThreadId();
	connect(this, &LogWindowTest::scrolltable, logView->mainView_.data(), [logView](QModelIndex index) {
//		log_trace(0) << "thread" << currentThreadId();
		logView->mainView_->scrollTo(index);
	});
			
	connect(&scrollTimer, &QTimer::timeout, this, [this, &logView]() {
		auto pos = rand() % 40000;
		auto model = logView->model();
		//QModelIndex index = model->currentIndex();
		auto index = logView->model()->index(pos, 0);
		//model->setCurrentIndex(index);
		emit scrolltable(index);
		//logView->_logView->scrollTo(index);
	});
	scrollTimer.start(100);
	exec();
}

LogWindowTest::LogWindowTest()
{
	moveToThread(this);
	start();
}
#endif