#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QColor>
#include <QFont>
#include <QPoint>
#include <QSize>
#include <QList>
#include <QDebug>
#include <map>
#include <logger/Logger.h>
#include "forms/colorpicker.h"
Q_DECLARE_METATYPE(ColorList)

template<class C>
class Allocator {
	std::map<void *, C *> _omap;
public:
	template<class T>
	bool validateObject(T *&o) {
		return _omap.find(&o) != _omap.end();
	}
	template<class T>
	bool allocateObjectOnce(T *&o) {
		if(!validateObject<T>(o)) {
			o = new T;
			_omap[&o] = o;
		}
		return true;
	}
	template<class T, typename ARG1>
	bool allocateObjectOnce(T *&o, ARG1 arg1) {
		if(!validateObject<T>(o)) {
			o = new T(arg1);
			_omap[&o] = o;
		}
		return true;
	}

};
class PropBaseClass
{
protected:
	Allocator<PropBaseClass> _objectAllocator;
	QString _basePath;
	QString _group;
	PropBaseClass *_parent;
	PropBaseClass *__castToProp() { return this; }
public:
	void setBasePath(const QString& path) {
		if(path.length())
			_basePath = "/" + path;
	}
	QString getPath() {
		return path();
	}

	PropBaseClass(PropBaseClass *parent = NULL) { _parent = parent; }
	void setSubPath(const QString& group) {
		_group = "/" + group;
	}
protected:
	PropBaseClass *root() {
		PropBaseClass *p = this;
		while(p && p->_parent)
			p = p->_parent;
		return p;
	}


	const QString path() const {
		QString p = _parent ? _parent->path() + "/" : _basePath;
		p += name() + _group;
		return p;
	}
public:
	virtual void set(const QString& name, const QVariant& value) {
		root()->set(path() + "/" + name, value); }

	virtual QVariant get(const QString& name, const QVariant& def = QVariant()) {
		return root()->get(path() + "/" + name, def); }
	virtual void remove(const QString& name = "") {
		if(_parent) _parent->remove(name.length() == 0 ? path() : path() + "/" + name);
	}

	virtual const QString name() const { return ""; }
};




#define XPROPCLASS_BEGIN(NAME, BASE) \
public:\
class NAME##Class: public BASE##Class {\
	private: const QString name() const { return #NAME; } \
	public:\
		NAME##Class(PropBaseClass *parent): BASE##Class(parent) {}\
		void remove() { BASE##Class::remove(); }

#define XPROPCLASS_END(NAME)  }; \
	private: NAME##Class *_##NAME; \
	public: NAME##Class& NAME() { \
		_objectAllocator.allocateObjectOnce<NAME##Class,PropBaseClass*>(_##NAME, this->__castToProp());\
		return *_##NAME;\
	}


#define PROPCLASS_BEGIN(NAME) \
	XPROPCLASS_BEGIN(NAME, PropBase)

#define PROPCLASS_END(NAME) \
	XPROPCLASS_END(NAME)

#define XPROPLIST_BEGIN(NAME, BASE) \
	XPROPCLASS_BEGIN(NAME, BASE)

#define XPROPLIST_END(NAME) \
	PROPLIST_END(NAME)


#define PROPLIST_BEGIN(NAME) \
	PROPCLASS_BEGIN(NAME)

#define PROPLIST_END(NAME) \
	PROPCLASS_END(NAME)\
	public: NAME##Class& NAME(const QString& group) { NAME().setSubPath(group); return NAME(); } \
	public: NAME##Class& NAME(int group) { NAME().setSubPath(QString("%1").arg(group)); return NAME(); }


#define PROPLIST(TYPE, NAME) \
	PROPCLASS(TYPE, NAME) \
	TYPE##Class& NAME(const QString& group) { NAME().setSubPath(group); return NAME(); } \
	TYPE##Class& NAME(int group) { NAME().setSubPath(QString("%1").arg(group)); return NAME(); }

#define PROPCLASS(TYPE, NAME) \
	private: TYPE##Class *_##NAME; \
	public: TYPE##Class& NAME() { \
		_objectAllocator.allocateObjectOnce<TYPE##Class,PropBaseClass*>(_##NAME, this->__castToProp());\
		return *_##NAME;\
	}

#define PROP(TYPE, NAME) \
private: TYPE _##NAME; \
public: TYPE NAME##() { return qvariant_cast<TYPE>(get(#NAME));} \
public: void NAME##(TYPE v) { _##NAME = v; QVariant variant; variant.setValue(v); set(#NAME, variant);}

#define PROPUSE(OWNER) \
	Allocator _objectAllocator;\
	class PropBaseClass;\
	private: PropBaseClass *__castToProp() { return &_propBase; }\
	public: void setBasePath(const QString& basePath) { _propBase.setBasePath(basePath); }\
	PropBaseClass _propBase;

/*typedef QList<QColor> ColorList;
Q_DECLARE_METATYPE(ColorList);*/

class Settings : public QSettings, public PropBaseClass
{
	Q_OBJECT
public:
	virtual void set(const QString& name, const QVariant& value)
	{
		/*qDebug("set %s value=%s",
		   name.toStdString().c_str(),
		   value.toString().toStdString().c_str());*/
		setValue(name, value);
	}
	virtual QVariant get(const QString& name, const QVariant& def)
	{
		QVariant value = QSettings::value(name, def);
		/*qDebug("read %s value=%s",
		   name.toStdString().c_str(),
		   value.toString().toStdString().c_str());*/
		return value;
	}
	void remove(const QString& path)
	{
		qDebug() << "remove" +  path;
		QSettings::remove(path);
	}

private:
	static QString _organisation;
	static QString _application;
public:
	static void setOrganisation(const QString organisation);
	static void setApplication(const QString application);

public:
	Settings(QString basePath = "");
	QStringList childGroups(const QString& group);
	QStringList childKeys(const QString& group);

	PROPCLASS_BEGIN(connections)
		PROPLIST_BEGIN(database)
			PROP(QString, driver)
			PROP(QString, database)
			PROP(QString, host)
			PROP(QString, username)
			PROP(QString, password)
		PROPLIST_END(database)
		PROPLIST_BEGIN(logstash)
			PROP(QString, host)
			PROP(QString, username)
			PROP(QString, password)
		PROPLIST_END(logstash)
	PROPCLASS_END(connections)

	PROPLIST_BEGIN(columnizer)
		PROP(QString, startPattern)
		PROP(QString, subject)
		PROP(QByteArray, tableState)
		PROPLIST_BEGIN(columns)
			PROP(QString, name)
			PROP(QString, pattern)
			PROP(QString, format)
			PROP(bool, enabled)
			PROP(QString, fmtFunc)
			PROP(QString, fmtFrom)
			PROP(QString, fmtTo)
		PROPLIST_END(columns)
	PROPLIST_END(columnizer)
	PROPLIST_BEGIN(filters)
		PROP(int, limit)
		PROP(QString, sql)
	PROPLIST_END(filters)

	PROPCLASS_BEGIN(queryConditions)
		PROP(QString, fileName)
		PROP(QString, connectionName)
		PROP(QString, tableName)
		PROP(QString, queryString)
		PROP(int, limit)
	PROPCLASS_END(queryConditions)

	PROPCLASS_BEGIN(model)
		PROP(QString, className)

	PROPCLASS_END(model)

	PROPCLASS_BEGIN(window)
		PROP(QByteArray, viewState)
		PROP(QPoint, pos)
		PROP(QSize, size)
		PROP(bool, fullScreen)
		PROP(bool, maximized)
		PROP(bool, minimized)
		PROP(QString, title)
		PROP(QFont, font)
		PROP(int, fontSize)
	PROPCLASS_END(window)

	XPROPCLASS_BEGIN(mainWindow, window)
		PROP(bool, tabbedView)
	XPROPCLASS_END(mainWindow)

	PROPLIST_BEGIN(column)
		PROP(int, colorize)
		PROP(int, visible)
		PROP(int, width)
		PROP(bool, visibleDetail)
		PROP(ColorList, availableCellColors)
	PROPLIST_END(column)

	PROPCLASS_BEGIN(rowStyle)
		PROPLIST_BEGIN(textColorizer)
			PROP(QString, textPart)
			PROP(QColor, textPartColor)
		PROPLIST_END(textColorizer)
		PROP(bool, alternatingRowColors)
	PROPCLASS_END(rowStyle)

	XPROPCLASS_BEGIN(detail, window)
		PROPLIST(column, columns)
	XPROPCLASS_END(detail)
#if 1
	XPROPCLASS_BEGIN(view, window)
		PROP(bool, followMode)
		PROP(bool, alternatingRowColors)
		PROP(QByteArray, splitter)
		PROPLIST(column, columns)
		PROPCLASS(queryConditions, queryConditions)
		PROPCLASS(rowStyle, rowStyle)
		PROP(ColorList, availableRowColors)
	XPROPCLASS_END(view)

	XPROPCLASS_BEGIN(logView, window)
		PROPCLASS(view, view)
		PROPCLASS(detail, detail)
	XPROPCLASS_END(logView)
	XPROPLIST_BEGIN(views, logView)
	XPROPLIST_END(views)
#else
	XPROPCLASS_BEGIN(view, window)
		PROP(bool, followMode)
		PROP(bool, alternatingRowColors)
		PROP(QByteArray, splitter)
		PROPLIST(column, columns)
		PROPCLASS(QueryOptions, QueryOptions)
		PROPCLASS(rowStyle, rowStyle)
		PROPCLASS(detail, detail)
	XPROPCLASS_END(view)

	/*XPROPCLASS_BEGIN(logView, window)
		PROPCLASS(view, view)
		PROPCLASS(detail, detail)
	XPROPCLASS_END(logView)*/
	XPROPLIST_BEGIN(views, view)
	XPROPLIST_END(views)
#endif
	//XPROPLIST_BEGIN(xviews, view)
	//XPROPLIST_END(xviews)

	PROPCLASS_BEGIN(logModel)
		PROPLIST(column, columns)
		PROPCLASS(queryConditions, queryConditions)
	PROPCLASS_END(logModel)

	PROPCLASS_BEGIN(general)
		PROPCLASS(view, view)
		PROPCLASS(queryConditions, queryConditions)
		PROP(int, logLevel)
		PROP(ColorList, availableColors)
	PROPCLASS_END(general)

	/**
	  windowTemplates settings are used to open views with predefined settings depends
	  on tableName, viewName etc.
	*/
	PROPLIST_BEGIN(windowTemplates)
		 PROP(QString, viewNameFilter) // this could be an regex to match with the viewname
		 PROP(QString, tableNameFilter)
		 PROPCLASS(logView, logView)
		 PROPCLASS(rowStyle, rowStyle)
	 PROPLIST_END(windowTemplates)

signals:

public slots:

};

#endif // SETTINGS_H
