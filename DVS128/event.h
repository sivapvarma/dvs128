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
	uint64_t t;
	uint16_t x, y;
	uint8_t parity;
	uint8_t id;
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
