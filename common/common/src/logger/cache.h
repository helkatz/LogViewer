#include <ostream>
#include <sstream>
#include <memory>
#include <chrono>
#include <mutex>
#include <regex>
#include <ctime>

namespace logger {
	template<typename T>
	struct LoggerCache
	{
		T ti;
		int start, end;
		Logger *logger;
		LoggerCache()
		{
			clear();
		}

		Logger *get(T ti)
		{
			for (int i = 0; i < maxCache; i++) {
				if (lc[i].ti == ti)
					return *lc[i].logger;
				if (lc[i].ti == NULL) {
					lc[i].ti = ti;
					lc[i].logger = &Logger::_get(get_name(ti).c_str());
					return *lc[i].logger;
				}
			}
			for (int i = 0; i < maxCache; i++)
				lc[i].clear();
			lc[0].ti = ti;
			lc[0].logger = &Logger::_get(get_name(ti).c_str());
			return *lc[0].logger;
		}

		void add(T ti, Logger *logger)
		{

		}

		void clear()
		{
			ti = nullptr;
			logger = nullptr;
			start = end = 0;
		}
	};
}



