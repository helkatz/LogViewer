#pragma once
#include <interfaces/LogModel.h>
#include <core/common.h>
#include <QList>
#include <QSharedPointer>
#include <QWidget>
#include <QRegularExpression>
namespace plugin_factory
{
	class CORE_API LogOpener: public QObject
	{
		Q_OBJECT
	public:
		explicit LogOpener(QObject *parent = nullptr)
			: QObject(parent)
		{}
		virtual QString name() const = 0;
		virtual QWidget *createWidget(QWidget *parent) = 0;
		virtual QList<QueryParams> exec() = 0;
	};

	class CreatorBase: public QObject
	{
		QString category_;
		QString name_;
	public:
		const QString& category() const 
			{ return category_; }
		const QString& name() const 
			{ return name_; }
		virtual QObject* create(QObject *parent) = 0;
		virtual const type_info& type() const = 0;

		template<typename T>
		T create(const QObject *parent)
		{
			return qobjectcast<T>(create(parent));
		}

		CreatorBase(const QString& category, const QString& name)
			: category_(category)
			, name_(name)
		{}
	};

	template<typename T>
	struct Creator: public CreatorBase
	{
		using CreatorBase::CreatorBase;
		QObject* create(QObject *parent) override
		{
			QObject *object = new T(nullptr);
			//CreatorBase* ret;
			//return ret;
			return object;
		};

		const type_info& type() const override
		{
			return typeid(T);
		}
	};

	class CORE_API Factory
	{		
		static QList<QSharedPointer<CreatorBase>> registered_;

	protected:

	public:
		virtual ~Factory() {};

		template<typename T>
		void registerType(const QString& category, const QString& name)
		{
			registered_ << QSharedPointer<CreatorBase>(new Creator<T>{ category, name });
		}

		static QList<CreatorBase *>  Get(const QString& category, QVariant name = QVariant{})
		{
			QList<CreatorBase *> ret;
			for (auto& creator : registered_) {
				QRegularExpression re(QString("^%1(\\..*|$)").arg(category));
				if (re.match(creator->category()).hasMatch() == false)
					continue;
				if (!name.isNull() && name.toString() != creator->name())
					continue;
				ret.push_back(creator.data());
			}
			return ret;
		}


		template<typename T>
		static QList<T> CreateAll(const QString& category, const QObject *parent)
		{
			QList<T> ret;
			for (auto creator : Get<T>(category)) {
				ret << creator->create(parent);
			}
			return ret;
		}

		template<typename T>
		static T Create(const QString& category, const QString& name, QObject *parent)
		{
			auto creator = Get(category, name);
			if (creator.size() == 0)
				return nullptr;
			return qobject_cast<T>(creator.at(0)->create(parent));
		}
		static void LoadPlugins();
	};
}

QT_BEGIN_NAMESPACE
#define PruginFactory_iid "hk.LogViewer.PluginFactory"
Q_DECLARE_INTERFACE(plugin_factory::Factory, PruginFactory_iid)
QT_END_NAMESPACE
