#ifndef INCLUDE_DVS128_EVENTIO_HPP
#define INCLUDE_DVS128_EVENTIO_HPP

#include "Event.hpp"
#include <string>
#include <vector>

namespace dvs128
{

	void SaveEvents(const std::string& fn, const std::vector<Event>& v);

	std::vector<Event> LoadEvents(const std::string& fn);

}

#endif
