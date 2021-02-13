#include <ostream>
#include <sstream>
#include <memory>
#include <chrono>
#include <mutex>
#include <regex>
#include <ctime>
namespace logger {

	std::string get_name(type_info *ti)
	{
		std::string name = ti->name();
		// find space after class or struct the classname should start here
		size_t beginName = name.find(' ');
		// eliminate all spaces
		beginName = name.find_first_not_of(' ', beginName);
		// find the end of the class name
		size_t endName = name.find(' ', beginName + 1);
		name = name.substr(beginName, endName - beginName);
		return name;
	}

	class FunctionInformation
	{
	public:
		std::string className;
		int classNameStart;
		std::string functionName;
		int funcNameStart;
		std::string fileName;
		/**
		windows Class::function
		parses from
			ClassA::name::<lambda_95a85966b1826af90681766a54e37442>::operator ()
		*/
		FunctionInformation(const char *funcname, const char *fileinfo)
		{
			classNameStart = 0;
			std::string name = funcname;

			name = std::tr1::regex_replace(name,
				std::tr1::regex("(::<lambda.*>::operator.*)"),
				"");
			name = std::tr1::regex_replace(name,
				std::tr1::regex("<.*>"),
				"");
			//std::tr1::match_results<std::string::const_iterator> m;
			/*std::tr1::smatch m;
			std::tr1::regex_search(name, m,
				std::tr1::regex("(\\w+)")
			);*/
			std::tr1::regex r("(\\w+)");
			std::sregex_iterator start(name.begin(), name.end(), r), end;
			std::vector<std::pair<std::string, unsigned int>> matches;
			for (; start != end; start++) {
				auto match = *start; // dereference the iterator to get the match_result
				matches.push_back(std::pair<std::string, unsigned int>(match.str(), match.position()));
			}
			fileName = fileinfo;
			functionName = funcname;
			if (matches.size() == 0)
				return;
			// seek to functionname its before <lambda....>::operator
			auto it = matches.end() - 1;
			functionName = it->first;
			funcNameStart = it->second;
			if (it != matches.begin()) {
				it--;
				className = it->first;
				classNameStart = it->second;
			}
		}
	};
}