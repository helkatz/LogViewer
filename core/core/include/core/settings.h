#pragma once
#include <core/settings_new.h>
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



#if 0
#define SETTINGSCLASS(NAME, BASE, ...)		\
	class NAME: public BASE {				\
		using Base = BASE;					\
	public:									\
		/*using BASE::BASE;*/					\
		NAME(const QString& name, PrivatePtr parent): \
			BASE(name, parent) {} \
		NAME(): BASE() { configure(); }\
		NAME(const QString& basePath): BASE(basePath) {}\
		NAME(const NAME& other): BASE(other) {}\
	public:									\
		__VA_ARGS__							\
	};

#define _PROPCLASS(TYPE, NAME)				\
	public: TYPE NAME() {					\
		return TYPE(#NAME, impl_);	\
	}

#define _PROPLIST(TYPE, NAME)						\
	public: std::map<QString, TYPE> NAME##List() {			\
		return children<TYPE>(#NAME);				\
	}												\
	public: TYPE NAME() {							\
		return TYPE(#NAME, impl_);			\
	}												\
	public: TYPE NAME(const QString& group) {		\
		return TYPE(#NAME"/" + group, impl_);\
	}												\
	public: TYPE NAME(int group) {					\
		return NAME(QString("%1").arg(group));		\
	}


#define _CONFIGURE(...)								\
	protected: void configure() override {			\
		Base::configure();							\
		__VA_ARGS__;								\
	}

#define _BASEPATH(PATH)								\
	protected: QString basePath() const { return #PATH; }

#define PROP(TYPE, NAME, ...) \
	public: const TYPE NAME##() const { return qvariant_cast<TYPE>(get(#NAME, __VA_ARGS__));} \
	public: TYPE NAME##() { return qvariant_cast<TYPE>(get(#NAME, __VA_ARGS__));} \
	public: void NAME##(TYPE v) { QVariant variant; variant.setValue(v); set(#NAME, variant);}

class CORE_API PSettings : protected QSettings {
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
	std::map<QString, T> children(const char *name)
	{
		std::map<QString, T> ret;
		foreach(auto group, childGroups(name)) {
			ret[group] = T(QString(name) + "/" + group, impl_);
		}
		return ret;
	}

	QStringList childGroupsFromCache(const QString& path);

	void loadIntoCache(const QString& path);

	virtual void configure();

	void setPath(const QString& path);
public:
	virtual ~PSettings();

	PSettings();

	PSettings(const QString& basePath);

	PSettings(const PSettings& other);

	PSettings(const QString& name, PrivatePtr parent);
	
	PSettings& operator = (const PSettings&);

	QStringList childGroups(const QString path = "");

	QStringList childKeys();

	QString expand(const QString& name = "") const;

	void set(const QString& name, const QVariant& value);

	QVariant get(const QString& name, const QVariant& def = QVariant{}) const;

	void remove();

	void remove(const QString& name);

	QString name() const;

	static void setOrganisation(const QString organisation);

	static void setApplication(const QString application);

	void clearCache();

	void saveCache();

	template<typename T>
	T as() {
		T ret;
		ret.impl_ = impl_;
		return ret;
	}

	template<typename T>
	const T as() const {
		return const_cast<PSettings*>(this)->as<T>();
	}
};
class Settings : public PSettings
{
public:
	using PSettings::PSettings;
	Settings();
};
namespace _settings {

	SETTINGSCLASS(Filter, PSettings,
		PROP(int, limit)
		PROP(QString, sql)
	)

	SETTINGSCLASS(QueryParams, PSettings,
		PROP(QString, connection)
		PROP(QString, modelClass)
		PROP(QString, queryString)
		PROP(quint64, fromTime, 0)
		PROP(quint64, toTime, std::numeric_limits<uint32_t>::max() * 1000ull)
		PROP(uint32_t, limit)
	);

	SETTINGSCLASS(Window, PSettings,
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

	SETTINGSCLASS(Column, PSettings,
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

	SETTINGSCLASS(TextColorizer, PSettings,
		PROP(QString, text)
		PROP(QFont, font)
		PROP(bool, useRegex)
		PROP(bool, wordOnly)
		PROP(bool, wholeRow)
		PROP(bool, caseSensitive)
		PROP(QColor, foregroundColor)
		PROP(QColor, backgroundColor)
	);

	SETTINGSCLASS(RowStyle, PSettings,
		_PROPLIST(TextColorizer, textColorizer)
		PROP(bool, alternatingRowColors)
	);

	SETTINGSCLASS(LogWindow, Window,
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
	SETTINGSCLASS(LogWindowTemplates, PSettings,
		PROP(QString, viewNameFilter) /* this could be an regex to match with the viewname */
		PROP(QString, tableNameFilter)
		_PROPCLASS(LogWindow, logView)
		_PROPCLASS(RowStyle, rowStyle)
	)

	SETTINGSCLASS(Common, PSettings,
		PROP(int, logLevel)
		PROP(QString, logFile)
		PROP(types::ColorList, availableColors)
	)

	SETTINGSCLASS(ApplicationSettings, Settings,
		_PROPCLASS(Common, general)
		_PROPLIST(Filter, filters)
		_PROPLIST(LogWindow, windows)
		_PROPCLASS(MainWindow, mainWindow)
		_PROPLIST(LogWindowTemplates, logWindowTemplates)
	)
}

template<typename T = _settings::ApplicationSettings>
T& settings() {
	static T settings;
	return settings;
}

#endif