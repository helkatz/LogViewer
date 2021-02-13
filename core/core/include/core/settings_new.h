#pragma once
#include <core/common.h>
#include <core/types.h>

#include <QSettings>
#include <QColor>
#include <QFont>
#include <QPoint>
#include <QSize>
#include <QList>
#include <QDebug>
#include <QMap>
#include <map>
#include <common/Logger.h>




#define SETTINGSCLASS(NAME, BASE, ...)		\
	class NAME: public BASE {				\
		using Base = BASE;					\
	public:									\
		/*using BASE::BASE;*/					\
		NAME(const QString& name, PrivatePtr parent): \
			BASE(name, parent) {} \
		NAME(): BASE() { configure(); }\
		NAME(const QString& basePath): BASE(basePath) {}\
		NAME(const BASE& other): BASE(other) {	\
			configure();						\
		}\
	public:									\
		__VA_ARGS__							\
	};

#define _PROPCLASS(TYPE, NAME)				\
	public: TYPE NAME() {					\
		return TYPE(#NAME, impl_);	\
	}

#define _XPROPLIST(TYPE, NAME)						\
	public: std::map<QString, TYPE> NAME##List() {	\
		return children_<TYPE>(#NAME);					\
	}												\
	public: PropList<TYPE> NAME() {							\
		return PropList<TYPE>(#NAME, impl_);					\
	}												\
	public: PropList<TYPE> NAME(const QString& group) {		\
		return PropList<TYPE>(#NAME"/" + group, impl_);		\
	}												\
	public: PropList<TYPE> NAME(int group) {					\
		return NAME(QString("%1").arg(group));		\
	}
#define _PROPLIST(TYPE, NAME)						\
	public: std::map<QString, TYPE> NAME##List() {	\
		return children_<TYPE>(#NAME);					\
	}												\
	public: TYPE NAME() {							\
		return TYPE(#NAME, impl_);					\
	}												\
	public: TYPE NAME(const QString& group) {		\
		return TYPE(#NAME"/" + group, impl_);		\
	}												\
	public: TYPE NAME(int group) {					\
		return NAME(QString("%1").arg(group));		\
	}

#define _CONFIGURE(...)								\
	protected: void configure() override {			\
		__VA_ARGS__;								\
		Base::configure();							\
	}

#define _BASEPATH(PATH)								\
	protected: QString basePath() const { return #PATH; }

#define PROP(TYPE, NAME, ...) \
	public: const TYPE NAME##() const { return qvariant_cast<TYPE>(get(#NAME, __VA_ARGS__));} \
	public: TYPE NAME##() { return qvariant_cast<TYPE>(get(#NAME, __VA_ARGS__));} \
	public: void NAME##(TYPE v) { QVariant variant; variant.setValue(v); set(#NAME, variant);}


class CORE_API PropClass {
protected:
	struct StorageData {
		bool isDirty = false;
		bool isRemoved = false;
		bool isGroup = false;
		QVariant value;
	};

	using StorageMap = QMap<QString, StorageData>;
	struct Private;
	using PrivatePtr = std::shared_ptr<Private>;
	PrivatePtr impl_;

	QString path() const;

	template<typename T>
	std::map<QString, T> children_(const char *name)
	{
		T self(QString(name), impl_);

		std::map<QString, T> ret;
		foreach(auto group, self.childGroups()) {
			ret[group] = T(QString(name) + "/" + group, impl_);
		}
		return ret;
	}

	virtual void configure();

	void setPath(const QString& path);
public:
	virtual ~PropClass();

	PropClass();

	PropClass(const QString& basePath);

	PropClass(const PropClass& other);

	PropClass(const QString& name, PrivatePtr parent);
	
	PropClass& operator = (const PropClass&);

	enum class FetchKeysMode { Keys, Groups, All };
	QStringList keys(const QString& path, FetchKeysMode mode) const;

	QStringList childGroups() const;

	QStringList childKeys(FetchKeysMode mode) const;

	QString expand(const QString& name = "") const;

	void set(const PropClass& other);

	void set(const QString& name, const QVariant& value);

	QVariant get(const QString& name, const QVariant& def = QVariant{}) const;

	void remove();

	void remove(const QString& name);

	QString name() const;

	void clearCache();

	void saveCache();

	void configurePersistent(const QString& organisation
		, const QString& application);

	void bindTo(PropClass& other);
	void unbind();

	template<typename T>
	T as() {
		T ret;
		ret.bindTo(*this);
		//ret.impl_ = impl_;
		return ret;
	}

	template<typename T>
	const T as() const {
		return const_cast<PropClass*>(this)->as<T>();
	}
};

template<typename T>
class PropList: public T, private QMap<QString, T>
{
	friend T;
	QMap<QString, T> list_;
public:
	using T::T;
	//std::map<QString, T> children()
	//{
	//	std::map<QString, T> ret;
	//	foreach(auto group, childGroups()) {
	//		ret[group] = T(QString(name) + "/" + group, impl_);
	//	}
	//	return ret;
	//}
	auto begin() {		
		for(auto group: childGroups()) {
			list_[name] = T(QString(name) + "/" + group, impl_);
		}
		return list_.begin();
	}
};
class CORE_API Settings : public PropClass
{
	void loadIntoCache(const QString& path);
	void loadIntoLocalStorage();

protected:
	void configure() override;
public:
	using PropClass::PropClass;

	static void setOrganisation(const QString& organisation);

	static void setApplication(const QString& application);

	Settings();
};
namespace settings_new {

	SETTINGSCLASS(Filter, PropClass,
		PROP(int, limit)
		PROP(QString, sql)
	)

	SETTINGSCLASS(QueryParams, PropClass,
		PROP(QString, connection)
		PROP(QString, modelClass)
		PROP(QString, queryString)
		PROP(quint64, fromTime, 0)
		PROP(quint64, toTime, std::numeric_limits<uint32_t>::max() * 1000ull)
		PROP(uint32_t, limit)
	);

	SETTINGSCLASS(Window, PropClass,
		PROP(QByteArray, viewState)
		PROP(QPoint, pos)
		PROP(QSize, size)
		PROP(bool, fullScreen)
		PROP(bool, maximized)
		PROP(bool, minimized)
		PROP(QString, title)
		PROP(QFont, font)
		PROP(int, fontSize)
	);

	SETTINGSCLASS(Column, PropClass,
		PROP(int, col)
		PROP(int, colorize)
		PROP(int, visible)
		PROP(int, width)
		PROP(bool, visibleDetail)
		PROP(types::ColorList, availableCellColors)
	);

	SETTINGSCLASS(MainWindow, Window,
		PROP(bool, tabbedView)
	);

	SETTINGSCLASS(TextColorizer, PropClass,
		PROP(QString, text)
		PROP(QFont, font)
		PROP(bool, useRegex)
		PROP(bool, wordOnly)
		PROP(bool, wholeRow)
		PROP(bool, caseSensitive)
		PROP(QColor, foregroundColor)
		PROP(QColor, backgroundColor)
	);

	SETTINGSCLASS(RowStyle, PropClass,
		_PROPLIST(TextColorizer, textColorizer)
		PROP(bool, alternatingRowColors)
	);

	SETTINGSCLASS(LogWindow, Window,
		SETTINGSCLASS(DetailView, PropClass,
			PROP(QFont, font)
			PROP(QStringList, visbible)
		)
		_PROPCLASS(DetailView, detailView)
		PROP(bool, followMode)
		PROP(bool, alternatingRowColors)
		PROP(QByteArray, splitter)
		_PROPLIST(Column, columns)
		_PROPCLASS(QueryParams, queryParams)
		_PROPCLASS(RowStyle, rowStyle)
		PROP(types::ColorList, availableRowColors)
		PROP(QByteArray, header)
	);

	/**
		windowTemplates settings are used to open views with predefined settings depends
		on tableName, viewName etc.
	*/
	SETTINGSCLASS(LogWindowTemplates, PropClass,
		PROP(QString, viewNameFilter) /* this could be an regex to match with the viewname */
		PROP(QString, tableNameFilter)
		_PROPCLASS(LogWindow, templateWindow)
	)

	SETTINGSCLASS(Common, PropClass,
		PROP(int, logLevel)
		PROP(QString, logFile)
		PROP(types::ColorList, availableColors)
	)

	SETTINGSCLASS(ApplicationSettings, PropClass,
		_CONFIGURE(
			setPath("");
			configurePersistent("ACOM", "LogViewer");
		)
		_PROPCLASS(Common, general)
		_PROPLIST(Filter, filters)
		_PROPLIST(LogWindow, windows)
		_PROPCLASS(MainWindow, mainWindow)
		_PROPLIST(LogWindowTemplates, logWindowTemplates)
	)
}
namespace _settings = settings_new;
template<typename T = settings_new::ApplicationSettings>

T& appSettings() {
	static T settings;
	return settings;
}

