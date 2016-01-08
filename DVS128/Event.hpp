#ifndef INCLUDE_DVS128_EVENT_HPP_
#define INCLUDE_DVS128_EVENT_HPP_

#include "event.h"
#include <stdint.h>
#include <iostream>

namespace dvs128
{
	typedef dvs128_event_t Event;
}

inline std::ostream& operator<<(std::ostream& os, const dvs128::Event& e)
{
	os << "[t=" << e.t
		<< ", id=" << static_cast<unsigned int>(e.id)
		<< ", p=" << static_cast<unsigned int>(e.parity)
		<< ", (" << e.x << ", " << e.y << ")]";
	return os;
}

namespace dvs128
{
	/** A normal dvs128 event with high-resolution timestamp, device id and real coordinates
	 * Structure size is 21 bytes
	 */
	struct EventF
	{
		// timestamp
		uint64_t time;

		// device id
		uint32_t id;

		// event pixel coordinate
		float x, y;

		// parity
		bool parity;

		friend inline std::ostream& operator<<(std::ostream& os, const dvs128::EventF& e) {
			os << "[t=" << e.time
				<< ", id=" << e.id
				<< ", p=" << e.parity
				<< ", (" << e.x << ", " << e.y << ")]";
			return os;
		}

	};

}

#endif
