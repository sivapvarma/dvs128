#include "EventIO.hpp"
#include "dvs128.h"
#include <stdio.h>

namespace dvs128
{
	void SaveEvents(const std::string& fn, const std::vector<Event>& v)
	{
		FILE* f = fopen(fn.c_str(), "w");
		if(f == 0) {
			std::cerr << "Error opening file '" << fn << "'!" << std::endl;
			return;
		}
		dvs128_file_write(f, v.data(), v.size());
		fclose(f);
	}

	std::vector<Event> LoadEvents(const std::string& fn)
	{
		std::vector<Event> v;
		FILE* f = fopen(fn.c_str(), "r");
		if(f == 0) {
			std::cerr << "Error opening file '" << fn << "'!" << std::endl;
			return v;
		}
		const size_t num_max = 1024;
		std::vector<dvs128_event_t> buffer(num_max);
		while(true) {
			ssize_t m = dvs128_file_read(f, buffer.data(), num_max);
			v.insert(v.end(), buffer.begin(), buffer.begin() + m);
			if(m != num_max) {
				break;
			}
		}
		fclose(f);
		return v;
	}

}
