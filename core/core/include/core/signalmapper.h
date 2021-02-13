#ifndef SIGNALMAPPER_H
#define SIGNALMAPPER_H
#include <core/common.h>
#include <QObject>
class CORE_API SignalMapperBase: public QObject
{
    //Q_OBJECT
protected:
    //virtual void mapperFunc() {};
public:
    SignalMapperBase(QObject *parent = NULL): QObject(parent) { }
};

#define SIGNALMAPPER1(NAME, TYPE, ...) \
class __##NAME: public SignalMapperBase \
{ \
    Q_OBJECT \
protected: TYPE _SPRM1;\
public slots: void slot(TYPE p1) { _SPRM1 = p1; mapperFunc(); }\
signals: void mappedSlot(##__VA_ARGS__##);\
};
#define SIGNALMAPPER1_MAP(NAME, ...) \
class NAME: public __##NAME \
{ \
    Q_OBJECT\
    virtual void mapperFunc() { \
        emit mappedSlot(##__VA_ARGS__##);\
    } \
};

#define SIGNALMAPPER_BEGIN(NAME) \
class NAME: public SignalMapperBase \
{ \
    Q_OBJECT

#define INP(TYPE, NAME) protected: TYPE NAME;

#define MAPFROM(...) \
    public slots: void slot(int inp1)

#define MAPTO(...) { emit mappedSlot(1, inp1); }

#define MAPTYPE(...) \
signals: void mappedSlot(int, int);

#define SIGNALMAPPER_END() };
class SMapInt_IntInt: public SignalMapperBase \
{ \
    Q_OBJECT
public:
    SMapInt_IntInt(QObject *parent = NULL):
        SignalMapperBase(parent) {}
    int prm1;
    void docon(const QObject *sender, const char *signal_index,
                            const QObject *receiver, const char * method_index,
                            Qt::ConnectionType type = Qt::AutoConnection) {
        connect(sender, signal_index, this, SLOT(slot(int)));
        connect(this, SIGNAL(mappedSlot(int,int)), receiver, method_index, type);
    }
signals: void mappedSlot(int, int);
public slots: void slot(int inp1) { emit mappedSlot(inp1, prm1); }
};

class SMapBool_BoolInt: public SignalMapperBase \
{ \
    Q_OBJECT
public:
    SMapBool_BoolInt(QObject *parent = NULL):
        SignalMapperBase(parent) {}

    int prm1;
    void docon(const QObject *sender, const char *signal_index,
                            const QObject *receiver, const char * method_index,
                            Qt::ConnectionType type = Qt::AutoConnection) {
        connect(sender, signal_index, this, SLOT(slot(bool)));
        connect(this, SIGNAL(mappedSlot(bool,int)), receiver, method_index, type);
    }
signals: void mappedSlot(bool, int);
public slots: void slot(bool inp1) { emit mappedSlot(inp1, prm1); }
};

class SMapInt_QString : public SignalMapperBase
{
Q_OBJECT
public:
	SMapInt_QString(QObject *parent = NULL) :
		SignalMapperBase(parent)
	{
	}

	void docon(const QObject *sender, const char *signal_index,
			   const QObject *receiver, const char * method_index,
			   Qt::ConnectionType type = Qt::AutoConnection)
	{
		connect(sender, signal_index, this, SLOT(slot(int)));
		connect(this, SIGNAL(mappedSlot(QString)), receiver, method_index, type);
	}
signals: 
	void mappedSlot(const QString&);
public slots: 
	void slot(int inp1) 
	{ 
		QString s; s.setNum(inp1);
		emit mappedSlot(s); 
	}
};
class SMapBool_QString : public SignalMapperBase
{
	Q_OBJECT
public:
	SMapBool_QString(QObject *parent = NULL) :
		SignalMapperBase(parent)
	{
	}

	void docon(const QObject *sender, const char *signal_index,
			   const QObject *receiver, const char * method_index,
			   Qt::ConnectionType type = Qt::AutoConnection)
	{
		connect(sender, signal_index, this, SLOT(slot(bool)));
		connect(this, SIGNAL(mappedSlot(QString)), receiver, method_index, type);
	}
signals:
	void mappedSlot(const QString&);
	public slots:
	void slot(bool inp1)
	{
		QString s; s.setNum(inp1);
		emit mappedSlot(s);
	}
};
#define MAPCONNECT(FROM) connect(FROM, )
SIGNALMAPPER_BEGIN(SMapInt_IntIntxx)
    MAPTYPE(int, int)
    MAPFROM(int inp1)
    MAPTO(1, inp1)
SIGNALMAPPER_END()

/*
#define SIGNALMAPPER1_MAP(NAME, ...) \
class NAME: public __##NAME \
{ \
    Q_OBJECT\
    virtual void mapperFunc() { \
        emit mappedSlot(##__VA_ARGS__##);\
    } \
};

SIGNALMAPPER1(SMapInt_IntInt, int, int, int)
SIGNALMAPPER1_MAP(SMapInt_IntInt, 10, _SPRM1)

SIGNALMAPPER1(SMapInt_QStringIntQString, int, QString, int, QString)
SIGNALMAPPER1_MAP(SMapInt_QStringIntQString, "prm1", _SPRM1, "prm2")
*/

#define SIGNALMAPPER_1_MAP(...) \
class SignalMapperMap: public SignalMapper \
{ \
    Q_OBJECT\
    virtual void mapperFunc() { \
        emit mappedSlot(##__VA_ARGS__##);\
    } \
};
class TestEmit: public QObject
{
    Q_OBJECT
signals:
    void signal1(int);

};


#endif // SIGNALMAPPER_H
