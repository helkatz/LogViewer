#if 0
#include <common/properties/Properties.h>
#include <Poco/Data/Session.h>
#include <gtest/gtest.h>
using namespace properties;

class Address : public Properties<Address>
{
	prop_ro(int, id);
	prop_rw(std::string, street);
	prop_rw(std::string, plz);

public:
	Address()
	{
		id = 10;
	}
};

struct Name
{
	Name(const char *) {}
};

template<typename NAME>
struct NameTest
{
	const char *name() { return typeid(*this).raw_name(); }
};
#include <regex>
TEST(T1, T1)
{
	std::string s = "dasdas__NameInfoStruct_thename_NameInfoStruct__dsda";
	std::regex re("/__NameInfoStruct_(.*)_NameInfoStruct__/");
	std::cmatch m;
	std::regex_match(s.c_str(),  m, re);

	std::cout << NameTest<struct Name1>().name() << std::endl;
	std::cout << NameTest<struct AnotherName>().name() << std::endl;
	std::cout << NameTest<struct Name11>().name() << std::endl;
}
class ClassWithProps : public Properties<ClassWithProps>
{
public:
	class TestVarBehavior: public DefaultBehavior<int>
	{
	public:
		static void init()
		{	
			defValue() = 2;
		}
	};

	class Inner : public Properties<Inner>
	{
		prop_rw(std::string, data);
	};
	prop_rw(Inner, inner);
	prop_rw(int, userId, defval = 2; fireEvents = true;);
	prop_rw(Address, address);
	prop_rw(std::string, username);
	prop_rw(Poco::DateTime, birthDate);
	//decl_prop(cpGlobals::eBrand::Enum, brandId);
};

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
struct TestSerializeData :public properties::Properties<TestSerializeData>
{
	prop_rw(int, intProp);

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		ar & intProp;
	}
};
TEST(PropertiesTest_2_0, serialize)
{

	std::stringstream ss;
#if 0
	{
		boost::archive::binary_oarchive ar(ss);

		TestSerializeData data;
		ar & data;
	}
	{
		boost::archive::binary_iarchive ar(ss);

		TestSerializeData data;
		ar & data;
		TestSerializeData data1;
		data1 = data;
	}
#endif
	//std::stringstream ss;
	{
		ss.clear();
		boost::archive::binary_oarchive ar(ss);

		std::map<int, TestSerializeData> data;
		data[0].intProp = 10;
		//data[1].intProp = 20;
		ar & data;
	}
	{
		std::cout << std::endl << "desirialize";
		boost::archive::binary_iarchive ar(ss);
		std::map<int, TestSerializeData> data, data1;
		ar & data;

		data1 = data;
	}
}


TEST(PropertiesTest_2_0, access_layer)
{
	class AccessTest : public Properties<AccessTest>
	{
	public:
		class Sub : public Properties<Sub>
		{
			prop_rw(int, readWriteInt);
		};
		prop_ro(Sub, subRO);
		prop_rw(Sub, subRW);

		/// only accessable within this class
		prop_prv(int, privateInt);

		/// only readable from outside
		prop_ro(int, readOnlyInt);

		/// read and writable from outside
		prop_rw(int, readWriteInt);
		AccessTest()
		{
			privateInt = 10;
			privateInt.set(10);
			privateInt.get();
			privateInt.clearModified();
			privateInt.initialized();
			privateInt.modified();
			readOnlyInt = 10;
		}
	};
	AccessTest props;

// only used to see compile errors
//#define CHECK_ACCESSABILITY
	// props->subRO = props->subRO; // compile error access denied
	props->subRW = props->subRW;
	// props->subRO = props->subRW;	// compile error access denied
	props->subRW = props->subRO;
	
	std::cout << props;
	std::stringstream ss;
	ss >> props;
	props.dump(ss);
	props->load(ss);
	// should not be possible because sub is readonly
#ifdef CHECK_ACCESSABILITY
	{
		int& iref = props->sub->readWriteInt.get();
		props->sub->readWriteInt = 10;// ->sub->readWriteInt = 10;
		props->sub->readWriteInt.set(10);
		props->sub->readWriteInt.clearModified();
		props->sub.set(props->sub);
		props->sub = props->sub;
	}
#endif
	// reading allowed
	{
		props->subRO->readWriteInt.modified();
		props->subRO->readWriteInt.initialized();		
	}

	// all not allowed compile error
#ifdef CHECK_ACCESSABILITY
	{
		props.privateInt = 10;
		props.privateInt.set(10);
		props.privateInt.modified();
		props.privateInt.initialized();
		props.privateInt.clearModified();
	}
#endif

	// modifing property not allowed compile error
#ifdef CHECK_ACCESSABILITY
	{
		int &iref = props.readOnlyInt.get();
		props.readOnlyInt = 10;
		props.readOnlyInt.set(10);
		props.readOnlyInt.clearModified();
	}
#endif

	// reading allowed
	{
		int ival = props.readOnlyInt.get();
		props.readOnlyInt.modified();
		props.readOnlyInt.initialized();
	}
	{
		// full access allowed
		int &iref = props.readWriteInt.get();
		props.readWriteInt = 10;
		props.readWriteInt.set(10);
		props.readWriteInt.clearModified();
		int ival = props.readOnlyInt.get();
		props.readWriteInt.modified();
		props.readWriteInt.initialized();
	}

	{
		ClassWithProps props;
		Address address;
		ASSERT_EQ(10, address->id.get());
		// address->id = 11;	// not allowed ro member
		// address->id.set(11); // not allowed ro member
		address->street = "10";
		props->address->street = "mystreet";
		//address.id = 10;
		//props.address->id = 1;
		//props.address.get().street = "mystreet";
		props.address.get().id;
	}
}

TEST(PropertiesTest_2_0, behaviors)
{
	ClassWithProps propClass;

}

TEST(PropertiesTest_2_0, access_operators)
{
	ClassWithProps propClass;
	propClass->address->street = "";
	propClass->address->street->length();
}
TEST(PropertiesTest_2_0, copy)
{
	class Test : public Properties<Test>
	{
		prop_rw(int, intProp);
		prop_rw(std::string, stringProp);
	};
	{
		Test props1;
		props1.intProp = 2;
		props1.stringProp = "string";
		Test props2 = props1;
		ASSERT_EQ(2, props2.intProp);
		ASSERT_EQ("string", props2.stringProp.get());
	}
	{
		Test props1, props2;
		props1.intProp = 2;
		props1.stringProp = "string";
		props2 = props1;
		ASSERT_EQ(2, props2.intProp);
		ASSERT_EQ("string", props2.stringProp.get());
	}
	{
		Test props1, props2;
		props1.intProp = 2;
		props1.stringProp = "string";
		props2.intProp = props1.intProp;
		props2.stringProp = props1.stringProp;
		ASSERT_EQ(2, props2.intProp);
		ASSERT_EQ("string", props2.stringProp.get());
	}

}

TEST(PropertiesTest_2_0, performance)
{
	class Test : public Properties<Test>
	{
		prop_rw(int, intProp);
		prop_rw(std::string, stringProp);
	};
	class TestNative
	{
		int intProp;
		std::string stringProp;
	};
	std::cout << " Props vs Native\n"
		<< "size " << sizeof(Test) << " vs " << sizeof(TestNative) << std::endl;
	auto ticksstart = GetTickCount();
	for (int i = 0; i < 100000; i++) {
		new Test;
	}
	auto time1 = GetTickCount() - ticksstart;
	ticksstart = GetTickCount();
	for (int i = 0; i < 100000; i++) {
		new TestNative;
	}
	auto time2 = GetTickCount() - ticksstart;
	std::cout << "times " << time1 << " vs " << time2 << std::endl;
}

TEST(PropertiesTest_2_0, all)
{
	ClassWithProps propClass;
	//Property<struct testVar, int> testVar;
	ASSERT_FALSE(propClass.userId.initialized());
	propClass.userId = 2;
	propClass.userId.set(1);
	std::cout << propClass << std::endl;
	ASSERT_TRUE(propClass.userId.initialized());
	ASSERT_TRUE(propClass.userId.modified());
	propClass.userId.clearModified();
	ASSERT_FALSE(propClass.userId.modified());
	ASSERT_EQ(1, propClass.userId.get());
	ASSERT_EQ("userId", propClass.userId.name());
	
	propClass->inner->data = "inner.data";
	ClassWithProps propClass2 = propClass;
	ASSERT_EQ("inner.data", propClass2->inner->data.get());
	//ASSERT_TRUE(propClass == propClass2);
	ASSERT_EQ(1, propClass2.userId.get());
	ASSERT_TRUE(propClass2.userId.initialized());
	propClass2.userId.set(2);
	std::cout << propClass << std::endl;
	std::cout << propClass2 << std::endl;
}

#include <poco/data/Connector.h>
//#include <Poco/Data/sqlite/Connector.h>
TEST(DISABLED_PropertiesTest_2_0, Poco_Data)
{
	using namespace Poco::Data::Keywords;
	namespace pdk = Poco::Data::Keywords;
	//Poco::Data::SQLite::registerConnector();
	//Poco::Data::S
	Poco::Data::Session sess("SQLite", "dummy.db");
	//Poco::Data::Session sess;// = dalayer::newSession("customer");
	struct Sub : public Properties<Sub>
	{
		prop_rw(std::string, stringVar);
	};
	class DataTest : public Properties<DataTest>
	{
	public:
		prop_ro(int, intVarRO);
		prop_rw(int, intVar);
		prop_rw(std::string, stringVar);
		prop_rw(Sub, sub);
		DataTest(Poco::Data::Session& sess) {
			sess << "select 1", into(intVarRO_()), now;			
		}
	};
	{
		DataTest data(sess);
		Property<int> prop;
		sess << "select 1, 'string', 'sub.string'", 
			into(data.intVar), into(data.stringVar), 
			into(data->sub->stringVar), now;
		ASSERT_EQ(1, data.intVar);
		ASSERT_EQ("string", data.stringVar.get());
		ASSERT_EQ(1, data.intVarRO);
	}
}
#endif