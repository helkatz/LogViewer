#include <gui/settings_dialog.h>
#include <core/types.h>
#include <core/settings_new.h>
#include <Windows.h>
#include <Winbase.h>
#include <gtest/gtest.h>
#include <QApplication>
#include <QRegularExpression>

SETTINGSCLASS(TestPropsInner, PropClass,
	PROP(int, intValue)
	PROP(QString, stringValue)
);

SETTINGSCLASS(BindTestProps, PropClass,
	_CONFIGURE(
		setPath("bindtest");
	)
	PROP(int, intValue)
	PROP(QString, stringValue)
	_PROPLIST(TestPropsInner, inners);

);

SETTINGSCLASS(TestProps, PropClass,
	PROP(int, intValue)
	PROP(QString, stringValue)
	_PROPCLASS(TestPropsInner, inner)
	_PROPLIST(TestPropsInner, inners)
);

SETTINGSCLASS(TestSettings, PropClass,
	_CONFIGURE(
		setPath("");
		configurePersistent("ACOM", "LogViewer_Test");
	)
	_PROPCLASS(TestProps, props);
	PROP(int, intValue);
	PROP(QString, stringValue);
	_PROPLIST(TestProps, testProps)

);

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
		auto s = TestSettings().props();
		s.intValue(10);
		s.inner().intValue(20);
		ASSERT_EQ(10, s.intValue());
		ASSERT_EQ(20, s.inner().intValue());
	}
	{
		auto s = TestSettings().props();
		ASSERT_EQ(10, s.intValue());
		ASSERT_EQ(20, s.inner().intValue());
	}
	{
		auto s = TestSettings().props();
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
		s.testProps(1).intValue(5);
		s.testProps(2).intValue(5);
		s.testProps(3).intValue(5);
		s.childKeys(PropClass::FetchKeysMode::Keys);
		s.childKeys(PropClass::FetchKeysMode::Groups);
		s.childKeys(PropClass::FetchKeysMode::All);
		ASSERT_EQ(3, s.testProps().childKeys(PropClass::FetchKeysMode::Groups).size());
	}
}

TEST(Settings, bindTo)
{
	TestSettings s;
	auto inner = s.testProps(1).inner();
	inner.intValue(1);
	inner.stringValue("s1");
	ASSERT_EQ(1, inner.intValue());
	s.testProps(1).inner().intValue(2);
	ASSERT_EQ(2, inner.intValue());
	inner.unbind();

	s.testProps(1).inner().intValue(3);
	// still 2 becasue its unbounded
	ASSERT_EQ(2, inner.intValue());
	inner.intValue(1);
	ASSERT_EQ(1, inner.intValue());
	ASSERT_EQ(3, s.testProps(1).inner().intValue());
	inner.bindTo(s.testProps(2));
	ASSERT_EQ(2, s.testProps(2).inner().intValue());
}
TEST(Settings, childGroups)
{
	{
		auto s = TestSettings().props();
		s.intValue(10);
		s.inner().intValue(20);
		ASSERT_EQ(10, s.intValue());
		ASSERT_EQ(20, s.inner().intValue());
	}
	{
		auto s = TestSettings().props();
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