#include <common/properties/Properties.h>
bool debug = false;
namespace properties
{
	std::mutex PropertiesBase::_mutex;
	UnusedOwner unusedOwner;
	UnusedOwner *PropertyBase::_globalOwner = &unusedOwner;

	PropertyBase::PropertyBase():
		_owner(_globalOwner) 
	{
		//dbg_prop() << "constzruct globalowner=" << addrname(_globalOwner);
		_owner->addProperty(this);
	};

	PropertyBase::PropertyBase(PropertiesBase *owner):
		_owner(owner)
	{
		//dbg_prop() << "constzruct owner="<< addrname(owner);
		_owner->addProperty(this);
	}

	PropertyBase::~PropertyBase()
	{
		//dbg_prop() << "destruct owner=" << addrname(_owner);
		_owner->removeProperty(this);
	}

	PropertiesBase& PropertiesBase::operator = (const PropertiesBase& other)
	{
		//dbg_props() << "assign other=" << addrname(&other);
		return *this;
	}

	PropertiesBase::PropertiesBase(const PropertiesBase& other)
	{
		//dbg_props() << "construct other=" << addrname(&other);
	}

	PropertiesBase::PropertiesBase()
	{
		//dbg_props() << "construct";
	}

	PropertiesBase::~PropertiesBase()
	{
		dbg_props() << "destruct";
	}

	void PropertiesBase::addProperty(PropertyBase *prop)
	{
		dbg_props();
		_properties.push_back(prop);
	}

	void PropertiesBase::removeProperty(PropertyBase *prop)
	{
		dbg_props();
		_properties.erase(
			std::find(_properties.begin(), _properties.end(), prop));
	}

	bool PropertiesBase::isModified() const
	{
		return std::find_if(_properties.begin(), _properties.end(), [](PropertyBase *prop) {
			return prop->modified();
		}) != _properties.end();
	}

	void PropertiesBase::clearModified() 
	{
		for(auto prop: _properties) {
			prop->clearModified();
		}
	}
}