#ifndef INCLUDED_EDVS_H
#define INCLUDED_EDVS_H

#include "event.h"
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>

// libcaer
#include <libcaer/libcaer.h>
#include <libcaer/devices/dvs128.h>

#ifdef __cplusplus
extern "C" {
#endif


// dvs128 device handle
// caerDeviceHandle dvs128_handle;

// dvs stream type
//typdef enum { DVS128_DEVICE_STREAM, DVS128_FILE_STREAM } stream_type;

struct dvs128_stream_t {
    // stream_type type;
    uintptr_t handle;
};

// typedef struct edvs_stream_t* edvs_stream_handle;
typedef struct dvs128_stream_t* dvs128_stream_handle;

/** Type of edvs connection */
typedef enum {
    EDVS_NETWORK_DEVICE, // network socket connection
    EDVS_SERIAL_DEVICE // serial port connection
} device_type;

/** Edvs device handle */
typedef struct {
    int handle;
    // device_type type;
    // caerDeviceHandle handle;
} dvs128_device_t;

/** Device streaming parameters and state */
typedef struct {
    dvs128_device_t* device;
    int device_timestamp_mode;
    int host_timestamp_mode;
    int master_slave_mode;
    unsigned char* buffer;
    size_t length;
    size_t offset;
//  uint64_t current_time;
//  uint64_t last_timestamp;
    uint64_t ts_last_device;
    uint64_t ts_last_host;
    uint64_t systime_offset;
} dvs128_device_streaming_t;

/** Opens an edvs event stream
 * URI can indicate three types of resources:
 *  Serial port:
 *          URI spec: ${PORT}?baudrate=${BAUDRATE}
 *          Example: /dev/ttyUSB0?baudrate=4000000
 *          Example: /dev/ttyUSB0 (assumes baudrate=4000000)
 *  File:
 *          URI spec: ${PATH}?dt=${DT}
 *          Example: /home/david/events?dt=50
 *          Example: /home/david/events (assumes dt=0)
 * @param uri URI to the event stream
 */
// edvs_stream_handle edvs_open(const char* uri);

/** Opens the device using libcaer
 * and return the handle to the device
 */
dvs128_stream_handle dvs128_open();


/** Runs the dvs128 event stream
 * This starts event transmission
 * @param return 0 on success and negative value on error
 */
int dvs128_run(dvs128_stream_handle h);

/** Checks if the end of an dvs128 event stream has been reached
 * @param return 1 if end of stream has been reached
 *          0 if end of stream not reached
 *          negative value on error
 */
int dvs128_eos(dvs128_stream_handle h);

/** Returns the master/slave mode
 * Returns 0 if normal mode (no master/slave)
 * Returns 1 if master
 * Returns 2 if slave
 */
int dvs128_get_master_slave_mode(dvs128_stream_handle h);

/** Checks if the stream is live
 * Returns 1 if the stream is a live stream (serial port / network)
 * Return 0 if the stream is not a live stream (file)
 */
int dvs128_is_live(dvs128_stream_handle h);

/** Closes an edvs event stream */
int dvs128_close(dvs128_stream_handle h);

/** Reads events from an edvs event stream
 * @param h stream handle
 * @param events buffer to store events (must have room for 'n' events)
 * @param n maximal number of events to read
 */
ssize_t dvs128_read(dvs128_stream_handle h, dvs128_event_t* events, size_t n);

ssize_t dvs128_read_ext(dvs128_stream_handle h, dvs128_event_t* events, size_t n, dvs128_special_t* special, size_t* ns);

ssize_t dvs128_write(dvs128_stream_handle h, const char* cmd, size_t n);


#ifdef __cplusplus
}
#endif

#endif
