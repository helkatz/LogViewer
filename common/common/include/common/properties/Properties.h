#pragma once
#include <common/StreamHelper.h>

#include <boost/variant.hpp>
#include <boost/optional.hpp>
//#include <boost/thread/tss.hpp>
//#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <boost/serialization/serialization.hpp>
#include <Poco/Data/AbstractExtraction.h>
#include <Poco/Data/TypeHandler.h>
#include <map>
#include <type_traits>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <regex>
extern bool debug;
inline std::string pretty_fnname(const char *name)
{
	std::regex re(".*::(.*)");
	std::cmatch m;
	if (std::regex_match(name, m, re) == false)
		return name;
	return m[1];
}
#define dbg_prop() if(debug) std::cout << std::endl << "Property::"<<pretty_fnname(__FUNCTION__) << " this=" << addrname(this) << " "
#define dbg_props() if(debug) std::cout << std::endl << "Properties::"<<pretty_fnname(__FUNCTION__) << " this=" << addrname(this) << " "
struct logstream: public std::stringstream
{
	~logstream()
	{
		std::cout << str() << std::endl;
	}
};

template<typename T>
inline std::string addrname(T* addr)
{
	return addrname((void*)addr);
}

inline std::string addrname(void* addr)
{
	static std::map<void *, int> addrMap;
	static int unique = 0;

	if (addrMap.find(addr) == addrMap.end())
		addrMap[addr] = unique++;
	return (boost::format("A_%1%(%2%)") % addrMap.at(addr) % addr).str();
}
#define DECLARE_EXCEPTION(NAME) \
class NAME : public std::exception \
{ \
public: \
	NAME() : std::exception() { } \
	NAME(const char *name) : std::exception(name) { } \
};

namespace properties
{
	namespace exceptions {
		DECLARE_EXCEPTION(PropertyNotAssignedException);
		DECLARE_EXCEPTION(PropertyNotInitializedException);
	}

	class UnusedOwner;
	class PropertiesBase;

	class PropertyBase
	{
		static UnusedOwner *_globalOwner;
	protected:
		PropertiesBase *_owner;

		PropertyBase();

		PropertyBase(PropertiesBase *owner);

		virtual ~PropertyBase();

	public:
		

		virtual PropertyBase *clone(PropertiesBase *owner) = 0;
	public:
		virtual void dump(std::ostream& os) const = 0;

		virtual void load(std::istream& is) = 0;

		virtual bool initialized() const = 0;

		virtual bool modified() const = 0;

		virtual void clearModified() = 0;

		virtual std::string name() const = 0;
	};

	class PropertiesBase
	{
		friend class PropertyBase;
		static std::mutex _mutex;
		PropertiesBase(const PropertiesBase&& other);
		PropertiesBase& operator = (const PropertiesBase&& other);
	protected:
		std::vector<PropertyBase *> _properties;

		PropertiesBase& operator = (const PropertiesBase& other);

		PropertiesBase(const PropertiesBase& other);

		PropertiesBase();

		virtual ~PropertiesBase();

		void addProperty(PropertyBase *prop);

		void removeProperty(PropertyBase *prop);

		PropertiesBase *getThis() { return this; }
	public:

		bool isModified() const;

		void clearModified();

		
	};

	template<typename T>
	class DefaultBehavior
	{
	public:
		static boost::optional<T>& defValue()
		{
			static boost::optional<T> defValue;
			return defValue;
		}
	public:
		static void init()
		{
		}
		static boost::optional<T>& getDefValue()
		{
			return defValue();
		}
		static void defValue(const T& v)
		{
			defValue() = v;
		}
	};

	class UnusedTemplateParam;
	class __NameInfoStruct_unknown_NameInfoStruct__;
	template<typename Type, typename NameInfoStruct = __NameInfoStruct_unknown_NameInfoStruct__, typename OwnerClass = UnusedOwner, typename Behavior = DefaultBehavior<Type>>
	class Property : private PropertyBase
	{
		using ValueType = Type;
		static const char fNone = 0x00;
		static const char fInitialized = 0x01;
		static const char fModified = 0x02;

		Type *_value{ nullptr };
		char _flags{ fNone };

		template<typename U=Type> typename std::enable_if<std::is_base_of<PropertiesBase, U>::value, void>::type
		clearModified_();

		template<typename U = Type> typename std::enable_if<!std::is_base_of<PropertiesBase, U>::value, void>::type
		clearModified_();
	protected:
		void dump(std::ostream& os) const override;

		void load(std::istream& is) override;

		PropertyBase *clone(PropertiesBase *cloneOwner) override;

	public:
		Property() {};

		Property(PropertiesBase *owner);

		Property(const Property& other);

		//Property(const Type& value);

		~Property();

		void reset();

		typename std::enable_if<!std::is_same<UnusedTemplateParam, OwnerClass>::value, OwnerClass&>::type
		set(const Type& value);
		
		const Type& get(boost::optional<Type> defVal = boost::none) const;

		Type& get(boost::optional<Type> defVal = boost::none);

		//operator Type& ();

		operator const Type& () const;

		bool initialized() const override;

		bool modified() const override;

		void clearModified() override;

		std::string name() const override;

		void operator = (const Property& other);

		void operator = (const Type& value);

		void operator = (const boost::optional<Type>& value);

		template<typename TargetType>
		const TargetType convert() const
		{
			std::stringstream iostr;
			dump(iostr);
			TargetType target;
			iostr >> target;
			return target;
		}

		const boost::optional<Type> toOptional() const
		{
			if (!initialized())
				return boost::none;
			return get();
		}
		const double toDouble() const { return convert < double >(); }
		const float toFloat() const { return convert < float >(); }
		const __int32 toInt() const { return convert < __int32 >(); }
		const __int8 toInt8() const { return convert < __int8 >(); }
		const __int16 toInt16() const { return convert < __int16 >(); }
		const __int32 toInt32() const { return convert < __int32 >(); }
		const __int64 toInt64() const { return convert < __int64 >(); }
		const char toChar() const { return convert < char >(); }
		const std::string toString() const { return convert < std::string >(); }
		const bool toBool() const { return convert < bool >(); }

		template<class Archive>
		void serialize(Archive & ar, const unsigned int version)
		{
			Type v{};
			if (initialized())
				v = get();
			ar & v;
			set(v);
		}
	};

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior = DefaultBehavior<Type>>
	class PropertyPrivate : protected Property<Type, NameInfoStruct, OwnerClass, Behavior>
	{
		using Property<Type, NameInfoStruct, OwnerClass, Behavior>::Property;
	protected:
		using Property<Type, NameInfoStruct, OwnerClass, Behavior>::operator =;

	public:
		friend OwnerClass;
	};

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior = DefaultBehavior<Type>>
	class PropertyRW : public Property<Type, NameInfoStruct, OwnerClass, Behavior>
	{
		using Property<Type, NameInfoStruct, OwnerClass, Behavior>::Property;
	public:
		friend OwnerClass;
		using Property<Type, NameInfoStruct, OwnerClass, Behavior>::operator =;

		Type* operator -> ()
		{
			return &get();
		}

		const Type* operator -> () const
		{
			return &get();
		}
	};

	
	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior = DefaultBehavior<Type>>
	class PropertyRO : public Property<Type, NameInfoStruct, OwnerClass, Behavior>
	{
		using Property<Type, NameInfoStruct, OwnerClass, Behavior>::Property;
	public:
		friend OwnerClass;
	protected:
		using Property<Type, NameInfoStruct, OwnerClass, Behavior>::clearModified;
		using Property<Type, NameInfoStruct, OwnerClass, Behavior>::set;
		using Property<Type, NameInfoStruct, OwnerClass, Behavior>::operator =;
		//operator = (const T& other){}

	public:
		const Type& get() const
		{
			return Property<Type, NameInfoStruct, OwnerClass, Behavior>::get();
		}

		const Type& operator -> () const
		{
			return get();
		}
	};

	template<typename Derived>
	class Properties : public PropertiesBase
	{
		// make some members private because PropertiesBase have to be public inherited
		// because of dynamic_cast in getOwner
		using PropertiesBase::addProperty;
		using PropertiesBase::removeProperty;

	public:
		Properties();

		Properties(const Properties& other);

	public:
		typedef Derived Derived;

		virtual ~Properties();

		void dump(std::ostream& os, const std::string& filter = "") const;

		void load(std::istream& is);

		friend std::ostream& operator << (std::ostream& os, const Properties<Derived>& o);

		friend std::istream& operator >> (std::istream& is, Properties<Derived>& o);

		bool operator == (const Properties<Derived>& rhs) const;

		bool operator != (const Properties<Derived>& rhs) const;

		void operator = (const Properties<Derived>& other);

		Derived* operator ->();

		const Derived* operator ->() const;

		friend std::ostream& operator << (std::ostream& os, const Derived& o)
		{
			o.dump(os);
			return os;
		}

		friend std::istream& operator >> (std::istream& is, Derived& o)
		{
			o.load(is);
			return is;
		}
	};

	class UnusedOwner : public Properties<UnusedOwner> {};
}

#define prop_ro(Type, Name) \
	protected: properties::PropertyRW<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived>& Name##_() { \
		return reinterpret_cast<properties::PropertyRW<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived>&>(Name); }\
	public: properties::PropertyRO<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived> Name = \
		properties::PropertyRO<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived>(getThis())

#define prop_rw(Type, Name) \
	public: properties::PropertyRW<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived> Name = \
		properties::PropertyRW<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived>(getThis())

#define prop_prv(Type, Name) \
	private: properties::PropertyRW<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived>& Name##_() { \
		return reinterpret_cast<properties::PropertyRW<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived>&>(Name); }\
	private: properties::PropertyPrivate<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived> Name = \
		properties::PropertyPrivate<Type, struct __NameInfoStruct_##Name##_NameInfoStruct__, Derived>{getThis()}

namespace properties {
	inline std::ostream& operator << (std::ostream& os, const Poco::DateTime&)
	{
		return os;
	}

	inline std::istream& operator >> (std::istream& is, Poco::DateTime& v)
	{
		return is;
	}

	template<typename T>
	typename std::enable_if<std::is_enum<T>::value, std::istream&>::type
		operator >> (std::istream& is, T& v)
	{
		return is;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline void Property<Type, NameInfoStruct, OwnerClass, Behavior>::dump(std::ostream& os) const
	{
		os << *_value;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline void Property<Type, NameInfoStruct, OwnerClass, Behavior>::load(std::istream& is)
	{
		Type v;
		is >> v;
		set(v);
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline
	PropertyBase *Property<Type, NameInfoStruct, OwnerClass, Behavior>::clone(PropertiesBase *cloneOwner)
	{
		dbg_prop() << "cloneOwner" << addrname(cloneOwner);
		PropertyBase *prop = this;
		size_t offset = (size_t)(void*)prop - (size_t)(void*)_owner;
		Property *clone = reinterpret_cast<Property *>((char*)cloneOwner + offset);
		clone = new(clone) Property(cloneOwner);
		return clone;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline
	Property<Type, NameInfoStruct, OwnerClass, Behavior>::Property(PropertiesBase *owner) :
		PropertyBase(owner),
		_value(nullptr),
		_flags(fNone)
	{
		dbg_prop() << " construct owner="<<addrname(owner);
		if (!std::is_same<UnusedTemplateParam, OwnerClass>::value) {
			if (std::is_base_of<PropertiesBase, Type>::value) {
				_value = new Type;
				_flags |= fInitialized;
			}
		}
		Behavior::init();
		if (Behavior::getDefValue())
			set(Behavior::getDefValue().get());
	}
#if 1
	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline Property<Type, NameInfoStruct, OwnerClass, Behavior>::Property(const Property& other):
		PropertyBase(_owner)
	{
		dbg_prop() << "copy other="<<addrname(&other);
		if (other._value)
			set(*other._value);
		_flags = other._flags;
	}
#endif
#if 0
	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline Property<Type, NameInfoStruct, OwnerClass, Behavior>::Property(const Type& value)
	{
		set(value);
	}
#endif
	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline Property<Type, NameInfoStruct, OwnerClass, Behavior>::~Property()
	{
		dbg_prop() << "destroy";
		//if(debug)dbg() << "  Property destroy  " << name() << " " << addrname(this) << " from " << addrname(_owner) << std::endl;
		reset();
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline void Property<Type, NameInfoStruct, OwnerClass, Behavior>::reset()
	{
		if (_value != nullptr)
			delete _value;
		_flags = 0;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline typename std::enable_if<!std::is_same<UnusedTemplateParam, OwnerClass>::value, OwnerClass&>::type
	Property<Type, NameInfoStruct, OwnerClass, Behavior>::set(const Type& value)
	{
		if (!_value) {
			_value = new Type;
			_flags |= fInitialized;
		}
		_flags |= (*_value != value ? fModified : _flags);
		*_value = value;

		return static_cast<OwnerClass &>(*_owner);
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline const Type& Property<Type, NameInfoStruct, OwnerClass, Behavior>::get(boost::optional<Type> defVal = boost::none) const
	{
		return *_value;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline Type& Property<Type, NameInfoStruct, OwnerClass, Behavior>::get(boost::optional<Type> defVal = boost::none)
	{
		return *_value;
	}
/*
	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline Property<Type, NameInfoStruct, OwnerClass, Behavior>::operator Type& ()
	{
		return get();
	}
*/
	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline Property<Type, NameInfoStruct, OwnerClass, Behavior>::operator const Type& () const
	{
		return get();
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline bool Property<Type, NameInfoStruct, OwnerClass, Behavior>::initialized() const
	{
		return (_flags & fInitialized) != 0;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline bool Property<Type, NameInfoStruct, OwnerClass, Behavior>::modified() const
	{
		return (_flags & fModified) != 0;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline void Property<Type, NameInfoStruct, OwnerClass, Behavior>::clearModified()
	{		
		clearModified_();
	}
#if 1
#pragma warning(push)
#pragma warning(disable:4544)
	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	template<typename U = Type> typename std::enable_if<std::is_base_of<PropertiesBase, U>::value, void>::type
	Property<Type, NameInfoStruct, OwnerClass, Behavior>::clearModified_()
	{
		typeid(U);
		if(_value)
			_value->clearModified();
		_flags &= ~fModified;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	template<typename U = Type> typename std::enable_if<!std::is_base_of<PropertiesBase, U>::value, void>::type
	Property<Type, NameInfoStruct, OwnerClass, Behavior>::clearModified_()
	{
		typeid(U);
		_flags &= ~fModified;
	}
#pragma warning(pop)
#endif
	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline std::string Property<Type, NameInfoStruct, OwnerClass, Behavior>::name() const
	{
		std::regex re(".*__NameInfoStruct_(.+?)_NameInfoStruct__.*");
		std::cmatch m;
		const char *name = typeid(*this).name();
		if (std::regex_match(name, m, re) == false)
			return "unknown";
		return m[1];
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline void Property<Type, NameInfoStruct, OwnerClass, Behavior>::operator = (const Property& other)
	{
		if (other._value)
			set(*other._value);
		_flags = other._flags;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline void Property<Type, NameInfoStruct, OwnerClass, Behavior>::operator = (const Type& value)
	{
		set(value);
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline void Property<Type, NameInfoStruct, OwnerClass, Behavior>::operator = (const boost::optional<Type>& value)
	{
		if (value)
			set(value.get());
		else
			reset();
	}


	template<typename Derived>
	inline Properties<Derived>::Properties()
	{
		dbg_props();
	}

	template<typename Derived>
	inline Properties<Derived>::Properties(const Properties& other) :
		PropertiesBase(other)
	{
		dbg_props();
		std::for_each(other._properties.begin(), other._properties.end(), [this](PropertyBase *prop) {
			prop->clone(this);
		});
		// this initializes the whole class so that each property inside will created correctly and
		// added to the _properties list
		//new(this) Derived;
	}

	template<typename Derived>
	inline Properties<Derived>::~Properties()
	{
		dbg_props();
	}

	template<typename Derived>
	inline void Properties<Derived>::dump(std::ostream& ss, const std::string& filter = "") const
	{
		auto findMaxLen = [this]() -> size_t {
			size_t maxLen = 0;
			std::for_each(_properties.begin(), _properties.end(), [&maxLen](PropertyBase *prop) {
				auto len = prop->name().length();
				if (maxLen < len)
					maxLen = len;
			});
			return maxLen;
		};
		// find out the max length of all prop names in this class to format the output pretty
		static size_t maxLen = findMaxLen();

		//std::stringstream ss;
		ss << typeid(Derived).name() << " (" << std::endl;
		{
			utilities::IndentingOStreambuf indention(ss, 4);
			std::for_each(_properties.begin(), _properties.end(), [&ss](PropertyBase *prop) {
				ss << std::setw(maxLen) << std::left;
				ss << prop->name() << ": [";
				//os << std::setw(3) << std::left;
				ss << (prop->initialized() ? "i" : "-");
				ss << (prop->modified() ? "m" : "-");
				ss << "] ";
				if (prop->initialized())
					prop->dump(ss);
				else
					ss << "not initialized";

				ss << std::endl;
			});
		}
		ss << "}";
		//return ss.str();
	}

	template<typename Derived>
	inline void Properties<Derived>::load(std::istream& is)
	{
		std::for_each(_properties.begin(), _properties.end(), [&is](PropertyBase *prop) {
			prop->load(is);
		});
	}

	template<typename Derived>
	inline bool Properties<Derived>::operator == (const Properties<Derived>& rhs) const
	{
		if (_properties.size() != rhs._properties.size())
			return false;
		return true;
	}

	template<typename Derived>
	inline bool Properties<Derived>::operator != (const Properties<Derived>& rhs) const
	{
		return !(*this == rhs);
	}

	template<typename Derived>
	void Properties<Derived>::operator = (const Properties<Derived>& other)
	{
		dbg_props() << "assign " << addrname(&other);
	}

	template<typename Derived>
	inline Derived* Properties<Derived>::operator ->()
	{
		return dynamic_cast<Derived*>(this);
	}

	template<typename Derived>
	inline const Derived* Properties<Derived>::operator ->() const
	{
		return dynamic_cast<const Derived*>(this);
	}

/******** Comparsion operators */

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline bool operator == (
		const Property<Type, NameInfoStruct, OwnerClass, Behavior>& lhs,
		const boost::optional<Type>& rhs)
	{
		return rhs.is_initialized() && rhs.get() == lhs;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline bool operator == (
		const boost::optional<Type>& lhs,
		const Property<Type, NameInfoStruct, OwnerClass, Behavior>& rhs)
	{
		return rhs == lhs;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline bool operator == (
		const Property<Type, NameInfoStruct, OwnerClass, Behavior>& lhs, 
		const Type& rhs)
	{
		return lhs.initialized() && lhs.get() == rhs; 
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline bool operator < (
		const Property<Type, NameInfoStruct, OwnerClass, Behavior>& lhs,
		const Type& rhs)
	{
		return lhs.initialized() == false || lhs.get() < rhs;
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline bool operator == (
		const Type& lhs, 
		const Property<Type, NameInfoStruct, OwnerClass, Behavior>& rhs)
	{
		return rhs.initialized() && lhs == rhs.get();
	}

	template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
	inline bool operator < (
		const Type& lhs, 
		const Property<Type, NameInfoStruct, OwnerClass, Behavior>& rhs)
	{
		return rhs.initialized() && lhs < rhs.get();
	}

}

namespace Poco
{
	namespace Data
	{
		template<typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
		class TypeHandler<properties::PropertyRW<Type, NameInfoStruct, OwnerClass, Behavior>>
		{
			typedef properties::PropertyRW<Type, NameInfoStruct, OwnerClass, Behavior> ObjType;
		public:
			static void bind(std::size_t pos, const ObjType& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
			{
				TypeHandler<Type>::bind(pos, obj.get(), pBinder, dir);
			}
			static std::size_t size()
			{
				return TypeHandler<Type>::size();
			}
			static void prepare(std::size_t pos, const ObjType& obj, AbstractPreparator::Ptr pPreparator)
			{
				TypeHandler<Type>::prepare(pos, obj.get(), pPreparator);
			}
			static void extract(std::size_t pos, ObjType& obj, const ObjType& defVal, AbstractExtractor::Ptr pExt)
			{
				Type value;
				TypeHandler<Type>::extract(pos, value, defVal.initialized() ? defVal.get() : Type{}, pExt);
				obj.set(value);
			}
		};
		template <typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
		class TypeHandler<properties::PropertyRO<Type, NameInfoStruct, OwnerClass, Behavior>>
		{
			typedef properties::PropertyRO<Type, NameInfoStruct, OwnerClass, Behavior> ObjType;
		public:
			static void bind(std::size_t pos, const ObjType& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
			{
				TypeHandler<Type>::bind(pos, obj.get(), pBinder, dir);
			}
			static std::size_t size()
			{
				return TypeHandler<Type>::size();
			}
			static void prepare(std::size_t pos, const ObjType& obj, AbstractPreparator::Ptr pPreparator)
			{
				TypeHandler<Type>::prepare(pos, obj.get(), pPreparator);
			}
			static void extract(std::size_t pos, ObjType& obj, const ObjType& defVal, AbstractExtractor::Ptr pExt)
			{
				Type value;
				TypeHandler<Type>::extract(pos, value, defVal.get(), pExt);
				obj.set(value);
			}
		};
		template < typename Type, typename NameInfoStruct, typename OwnerClass, typename Behavior>
		class TypeHandler<properties::PropertyPrivate<Type, NameInfoStruct, OwnerClass, Behavior>>
		{
			typedef properties::PropertyPrivate<Type, NameInfoStruct, OwnerClass, Behavior> ObjType;
		public:
			static void bind(std::size_t pos, const ObjType& obj, AbstractBinder::Ptr pBinder, AbstractBinder::Direction dir)
			{
				TypeHandler<Type>::bind(pos, obj.get(), pBinder, dir);
			}
			static std::size_t size()
			{
				return TypeHandler<Type>::size();
			}
			static void prepare(std::size_t pos, const ObjType& obj, AbstractPreparator::Ptr pPreparator)
			{
				TypeHandler<Type>::prepare(pos, obj.get(), pPreparator);
			}
			static void extract(std::size_t pos, ObjType& obj, const ObjType& defVal, AbstractExtractor::Ptr pExt)
			{
				Type value;
				TypeHandler<Type>::extract(pos, value, defVal.get(), pExt);
				obj.set(value);
			}
		};
	}
}

namespace boost {
	namespace serialization {
#if 0
		template<class Archive, typename Type, typename OwnerClass, typename Behavior>
		void serialize(Archive & ar, properties::PropertyRW<Type, OwnerClass, Behavior> & o, 
			const unsigned int version)
		{
			Type v{};
			if (o.initialized())
				v = o;
			ar & v;
			o.set(v);
		}
		template<class Archive, typename Type, typename OwnerClass, typename Behavior>
		void serialize(Archive & ar, properties::PropertyRO<Type, OwnerClass, Behavior> & o,
			const unsigned int version)
		{
			Type v{};
			if (o.initialized())
				v = o;
			ar & v;
			o.set(v);
		}
#endif
	} // namespace serialization

} // namespace boost
