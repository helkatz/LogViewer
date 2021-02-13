#pragma once
#include <exception>
#include "Utils/ScopedEnum.h"
SCOPED_ENUM(eError,
	
);
class Exception: public std::exception
{ 
	int _code;
	std::string _message;
public:
	Exception(int code, const std::string& msg);
};
#define DECLARE_EXCEPTION(NAME, MSG) \
	void throw##NAME() {\
		throw Exception(-1, MSG); \
	}

DECLARE_EXCEPTION(InvalidModelClass, "model class not valid")