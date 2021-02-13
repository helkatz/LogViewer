#include <common/properties/Properties.h>
#include <Poco/Data/Session.h>
#include <gtest/gtest.h>
using namespace properties;
class prop_base
{
public:
	//static std::vector<prop_base *> properties;
	virtual std::string name() const = 0;
	virtual size_t sizeof_this() const = 0;
	virtual bool is_modified() const = 0;
	virtual void clear_modified() = 0;
	//virtual std::string owner_name() const = 0;
};

class prop_base_ : public prop_base
{};

//std::vector<prop_base *> prop_base::properties;
struct NameInfoStructUnknown;
template<typename T, typename NameInfoStruct = NameInfoStructUnknown>
class prop : public boost::optional<T>, public prop_base_
{
	bool _modified{ false };

public:
	prop()
	{
	}
	template<typename R>
	prop(const R& other) :
		boost::optional<T>::optional(other)
	{
		_modified = is_initialized();
	}

	boost::optional<T>& operator = (const prop& rhs)
	{
		_modified = *this != rhs;
		return boost::optional<T>::operator = (rhs);
	}

	boost::optional<T>& operator = (const boost::optional<T>& rhs)
	{
		_modified = *this != rhs;
		return boost::optional<T>::operator = (rhs);
	}

	template<typename R>
	boost::optional<T>& operator = (const R& rhs)
	{
		_modified = !is_initialized() || this->value() != rhs;
		return boost::optional<T>::operator = (rhs);
	}

	bool is_modified() const override
	{
		return _modified;
	}

	void clear_modified() override
	{
		_modified = false;
	}

	std::string name() const override
	{
		std::string name = typeid(NameInfoStruct).name();
		const char *namePrefix = "NameInfoStruct";
		static size_t len = strlen(namePrefix);
		auto pos = name.find(namePrefix) + len;
		return name.substr(pos);
		return name.substr(7); // hide struct literal in name tested on windows c++98
	}

	size_t sizeof_this() const override
	{
		return sizeof(*this);
	}
};

template<typename Derived>
class properties_
{
	using propvec = std::vector<prop_base *>;
	propvec _propVec;

public:

	void initialize()
	{
		if (_propVec.size())
			return;
		std::string val;
		Derived *derived = dynamic_cast<Derived*>(this);
		auto thisSize = sizeof(*derived);
		for (int i = sizeof(this); i < thisSize;) {
			try {
				
				void *maybe = (void*)((char*)(this) + i);

				prop_base *p_base = reinterpret_cast<prop_base*>((void*)((char*)(this) + i));
				prop_base_ *p = dynamic_cast<prop_base_*>(p_base);
				if (p == nullptr)
					i += 4;
				else {
					std::cout << p->name() << " its a prop\n";
					i += p->sizeof_this();
					_propVec.push_back(p);
				}
			}
			catch (...) {
				i += 4;
			}
		}
	}
public:

	propvec::const_iterator begin() const
	{
		const_cast<properties_*>(this)->initialize();
		return _propVec.begin();;
	}

	propvec::const_iterator end() const
	{
		const_cast<properties_*>(this)->initialize();
		return _propVec.end();
	}

	virtual ~properties_() {}
	void clear_modified()
	{
		for (auto prop : *this) {
			prop->clear_modified();
		}
	}
};



class Sub
{
	prop<int, struct NameInfoStructintProp> intProp;
	prop<std::string, struct NameInfoStructstringProp> stringProp;
};

class Props: public properties_<Props>
{
public:
	prop<int, NameInfoStructintProp > intProp;
	int i;
	Sub sub;
	prop<std::string, NameInfoStructstringProp> stringProp;
};


TEST(PropertiesTestVariant1, prop)
{
	{
		Props props;
		props.stringProp = "hallo";
		ASSERT_TRUE(props.stringProp.is_modified());
		props.clear_modified();
		ASSERT_FALSE(props.stringProp.is_modified());
	}
	{

		boost::optional<int> propBO;
		prop<int> prop1 = propBO;
		prop1 = propBO;
		prop<int> prop2 = 3;
	}
	{
		boost::optional<int> propBO = 3;
		prop<int> prop1, prop2;
		prop1 = propBO;
		ASSERT_EQ(3, prop1);
		prop2 = prop1;
		ASSERT_EQ(3, prop2);

		prop2 = 2;
		ASSERT_EQ(2, prop2);
		propBO = prop2;
		ASSERT_EQ(2, propBO);
	}
	{
		boost::optional<int> propBO = 3;
		prop<int> prop1 = 3;
		prop<int> prop2 = propBO;
		ASSERT_EQ(3, prop1);
		ASSERT_EQ(3, prop2);
	}
	{
		boost::optional<int> propBO = 3;
		prop<int> prop1, prop2;
		ASSERT_FALSE(prop1.is_modified());
		prop1 = propBO;
		ASSERT_TRUE(prop1.is_modified());
	}
	{
		prop<int> prop1, prop2;
		ASSERT_FALSE(prop1.is_initialized());
		prop1 = 3;
		ASSERT_TRUE(prop1.is_initialized());
	}
}
