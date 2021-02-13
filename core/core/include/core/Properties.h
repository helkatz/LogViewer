#pragma once
#include <core/common.h>
#include <qmap.h>
#include <qvariant.h>
#include <QSharedPointer>

class CORE_API Properties
{	
protected:
    QSharedPointer<QMap<QString, QVariant>> _hive;

public:
    template<typename T>
    void set(const QString& name, const T& value)
    {
        (*_hive)[name] = value;
    }

    template<typename T>
    const T get(const QString& name) const
    {
        return qvariant_cast<T>((*_hive)[name]);
    }

    template<typename T>
    const T get(const QString& name, const T& def) const
    {
        return _hive->contains(name) ? get<T>(name) : def;
    }
public:
    Properties()
    {
        _hive = QSharedPointer<QMap<QString, QVariant>>(new QMap<QString, QVariant>);
    }

    Properties(const Properties& other)
    {
        *this = other;
    }

    virtual ~Properties()
    {

    }
};

#define PROPERTY(CLASS, TYPE, NAME, ...) \
    CLASS& NAME(const TYPE& value){ set<TYPE>(#NAME, value); return *this; } \
    TYPE NAME() const { return get<TYPE>(#NAME, __VA_ARGS__); }

	
