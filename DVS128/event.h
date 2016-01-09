#ifndef INCLUDED_DVS128_EVENT_H
#define INCLUDED_DVS128_EVENT_H

#include <stdint.h>
#include "string.h"

#ifdef __CPLUSPLUS__
extern "C" {
#endif

/** An dvs128 event
 * Struct uses 14 bytes of data.
 */
typedef struct {
	uint64_t t; // timestamp
	uint16_t x, y; // x, y coordinate of pixel
	uint8_t parity; // polarity 1/0
	uint8_t id; // device id
} dvs128_event_t;

/** An edvs special data block */
typedef struct {
	uint64_t t;
	size_t n;
	unsigned char data[16];
} dvs128_special_t;

#ifdef __CPLUSPLUS__
}
#endif

#endif
