#if 0
#include "PropertiesTest.h"
#include "Common/ScopedEnum.h"
using namespace common;

#include <initializer_list>
#include <algorithm>
template<class T> void f(std::initializer_list<T>);
TEST(MiscTest, T)
{
	//f({ 1 });
}

template<typename T>
class mytype
{
	T _value;
	bool _inititalized;
public:
	void putprop(const T& value)
	{
		_value = value;
	}
	T& getprop()
	{
		return _value;
	}
	__declspec(property(get = getprop, put = putprop)) T value;
};

void func1(int& i)
{
	i = 5;
}
#include <algorithm>
template<class ForwardIt, class T, class Compare = std::less<>>
ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value, Compare comp = {})
{
	first = std::lower_bound(first, last, value, comp);	
	return first != last && !comp(value, *first) ? first : last;
}
TEST(MiscTest, propertyTest)
{
	struct T {
		int min;
		int max;
	};

	std::vector<T> data2 = { { 0,3 },{ 4,9 },{ 10,13 },{ 19,20 } };
	T range = { 11,0 };

	auto lower = std::lower_bound(data2.begin(), data2.end(), range, [](const auto& a, const auto& b) {
		return a.min < b.min && a.max >= b.max;    
	});

	if (lower != data2.cend()) {
		//std::cout << it1->min << " " << it1->max << " found at index "<< std::distance(data2.cbegin(), it1);
	}
	auto it = binary_find(data2.begin(), data2.end(), range, [](const auto& a, const auto& b) {
		return a.min < b.min;// && a.max < b.max;    
	}); //< choosing '5' will return end()

	if (it != data2.cend())
		std::cout << it->min << " " << it->max << " found at index " << std::distance(data2.begin(), it);

}
#if 0
class Base
{
public:
	virtual void foo() = 0;
};
template <class T, bool = std::is_base_of<common::PropertiesBase, T>::value>
class B : public Base
{
public:
	void foo() override { std::cout << "A is not base of T!" << std::endl; }
};

template <class T>
class B<T, true> : public Base{
public:
	void foo() override { std::cout << "A is base of T!" << std::endl; }
};

class C : public Base
{

};

class D
{
};

TEST(MiscTest, Test2)
{
	B<C> b;
	B<D> b1;
}

#endif
#endif