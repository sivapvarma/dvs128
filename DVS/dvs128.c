#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "dvs128.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define DVS128_LOG_MESSAGE
#define DVS128_LOG_VERBOSE
// #define EDVS_LOG_ULTRA



#include <termios.h>
#include <stdio.h>
#include <fcntl.h>


void dvs128_serial_open(int id)
{
    // get a handle to device
    dvs128_handle = caerDeviceOpen(1, CAER_DEVICE_DVS128, 0, 0, NULL);
    // print device opened
    struct caer_dvs128_info dvs128_info = caerDVS128InfoGet(dvs128_handle);
    printf("%s --- ID: %d, Master: %d, DVS X: %d, DVS Y: %d, Logic: %d.\n", dvs128_info.deviceString,
        dvs128_info.deviceID, dvs128_info.deviceIsMaster, dvs128_info.dvsSizeX, dvs128_info.dvsSizeY,
        dvs128_info.logicVersion);
    // send default configuration
    caerDeviceSendDefaultConfig(dvs128_handle);
}

int edvs_serial_open(const char* path, int baudrate)
{
    int baudrate_enum;
    switch(baudrate) {
        case 2000000: baudrate_enum = B2000000; break;
        case 4000000: baudrate_enum = B4000000; break;
        default:
            printf("edvs_open: invalid baudrate '%d'!", baudrate);
            return -1;
    }

    int port = open(path, O_RDWR /*| O_NOCTTY/ * | O_NDELAY*/);
    if(port < 0) {
        printf("edvs_serial_open: open error %d\n", port);
        return -1;
    }
    // set baud rates and other options
    struct termios settings;
    if(tcgetattr(port, &settings) != 0) {
        printf("edvs_serial_open: tcgetattr error\n");
        return -1;
    }
    if(cfsetispeed(&settings, baudrate_enum) != 0) {
        printf("edvs_serial_open: cfsetispeed error\n");
        return -1;
    }
    if(cfsetospeed(&settings, baudrate_enum)) {
        printf("edvs_serial_open: cfsetospeed error\n");
        return -1;
    }
    settings.c_cflag = (settings.c_cflag & ~CSIZE) | CS8; // 8 bits
    settings.c_cflag |= CLOCAL | CREAD;
    settings.c_cflag |= CRTSCTS; // use hardware handshaking
    settings.c_iflag = IGNBRK;
    settings.c_oflag = 0;
    settings.c_lflag = 0;
    settings.c_cc[VMIN] = 1; // minimum number of characters to receive before satisfying the read.
    settings.c_cc[VTIME] = 5; // time between characters before satisfying the read.
    // write modified record of parameters to port
    if(tcsetattr(port, TCSANOW, &settings) != 0) {
        printf("edvs_serial_open: tcsetattr error\n");
        return -1;
    }
    return port;
}

// not sure what these functions do ???
ssize_t dvs128_serial_read()
{
    
}

ssize_t edvs_serial_read(int port, unsigned char* data, size_t n)
{
    ssize_t m = read(port, data, n);
    if(m < 0) {
        printf("edvs_serial_read: read error %zd\n", m);
        return -1;
    }
    return m;
}

ssize_t edvs_serial_write(int port, const char* data, size_t n)
{
    ssize_t m = write(port, data, n);
    if(m != n) {
        printf("edvs_serial_send: write error %zd\n", m);
        return -1;
    }
    return m;
}

int edvs_serial_close(int port)
{
    int r = close(port);
    if(r != 0) {
        printf("edvs_serial_close: close error %d\n", r);
        return -1;
    }
    return r;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

/** Reads data from an edvs device */
ssize_t edvs_device_read(edvs_device_t* dh, unsigned char* data, size_t n)
{
    return dvs128_serial_read(dh->handle, data, n);
    /*
    switch(dh->type) {
    case EDVS_NETWORK_DEVICE:
        return edvs_net_read(dh->handle, data, n);
    case EDVS_SERIAL_DEVICE:
        return edvs_serial_read(dh->handle, data, n);
    default:
        return -1;
    }
    */
}

/** Writes data to an edvs device */
ssize_t edvs_device_write(edvs_device_t* dh, const char* data, size_t n)
{
    return 1; // this is a huge hack
    /*
    switch(dh->type) {
    case EDVS_NETWORK_DEVICE:
        return edvs_net_write(dh->handle, data, n);
    case EDVS_SERIAL_DEVICE:
        return edvs_serial_write(dh->handle, data, n);
    default:
        return -1;
    }
    */
}

int edvs_device_write_str(edvs_device_t* dh, const char* str)
{
    return 0; // huge hack
    /*
    size_t n = strlen(str);
    if(edvs_device_write(dh, str, n) != n) {
        return -1;
    }
    else {
        return 0;
    }
    */
}


/** Closes an edvs device connection */
int edvs_device_close(edvs_device_t* dh)
{
    return dvs128_serial_close(dh->handle);
    /*
    switch(dh->type) {
    case EDVS_NETWORK_DEVICE:
        return edvs_net_close(dh->handle);
    case EDVS_SERIAL_DEVICE:
        return edvs_serial_close(dh->handle);
    default:
        return -1;
    }
    */
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

uint64_t timestamp_limit(int mode)
{
    switch(mode) {
        default: return 0; // no timestamps
        case 1: return (1ull<<16); // 16 bit
        case 2: return (1ull<<24); // 24 bit
        case 3: return (1ull<<32); // 32 bit
    }
}

uint64_t get_micro_time()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return 1000000ull*(uint64_t)(t.tv_sec) + (uint64_t)(t.tv_nsec)/1000ull;
}

void sleep_ms(unsigned long long milli_secs)
{
    struct timespec ts;
    ts.tv_sec = milli_secs / 1000L;
    ts.tv_nsec = (milli_secs * 1000000L) % 1000000000L;
    nanosleep(&ts, NULL);
}

uint64_t c_uint64_t_max = 0xFFFFFFFFFFFFFFFFL;

edvs_device_streaming_t* edvs_device_streaming_open(edvs_device_t* dh, int device_tsm, int host_tsm, int master_slave_mode)
{
    edvs_device_streaming_t *s = (edvs_device_streaming_t*)malloc(sizeof(edvs_device_streaming_t));
    if(s == 0) {
        return 0;
    }
    s->device = dh;
    s->device_timestamp_mode = device_tsm;
    s->host_timestamp_mode = host_tsm;
    s->master_slave_mode = master_slave_mode;
    s->length = 8192;
    s->buffer = (unsigned char*)malloc(s->length);
    s->offset = 0;
//  s->current_time = 0;
//  s->last_timestamp = timestamp_limit(s->device_timestamp_mode);
    s->ts_last_device = c_uint64_t_max;
    s->ts_last_host = s->ts_last_device;
    s->systime_offset = 0;
    // // reset device
    if(edvs_device_write_str(dh, "R\n") != 0)
        return 0;
    sleep_ms(200);
    // timestamp mode
    if(s->device_timestamp_mode == 1) {
        if(edvs_device_write_str(dh, "!E1\n") != 0)
            return 0;
    }
    else if(s->device_timestamp_mode == 2) {
        if(edvs_device_write_str(dh, "!E2\n") != 0)
            return 0;
    }
    else if(s->device_timestamp_mode == 3) {
        if(edvs_device_write_str(dh, "!E3\n") != 0)
            return 0;
    }
    else {
        if(edvs_device_write_str(dh, "!E0\n") != 0)
            return 0;
    }
    // master slave
    if(s->master_slave_mode == 1) {
        // master
#ifdef EDVS_LOG_MESSAGE
        printf("Arming master\n");
#endif
        if(edvs_device_write_str(dh, "!ETM0\n") != 0)
            return 0;
    }
    else if(s->master_slave_mode == 2) {
        // slave
#ifdef EDVS_LOG_MESSAGE
        printf("Arming slave\n");
#endif
        if(edvs_device_write_str(dh, "!ETS\n") != 0)
            return 0;
    }
    return s;
}

int flush_device(edvs_device_streaming_t* s)
{
    if(s->device->type == EDVS_SERIAL_DEVICE) {
        int port = s->device->handle;
        // set non-blocking
        int flags = fcntl(port, F_GETFL, 0);
        fcntl(port, F_SETFL, flags | O_NONBLOCK);
        // read everything
        unsigned char buff[1024];
        while(edvs_device_read(s->device, buff, 1024) > 0);
        // set blocking
        fcntl(port, F_SETFL, flags);
        return 0;
    }
    if(s->device->type == EDVS_NETWORK_DEVICE) {
        // FIXME
        printf("ERROR flush for network device not implemented\n");
        return -1;
    }
    printf("edvs_run: unknown stream type\n");
    return -1;
}

// str must be null terminated
int wait_for(edvs_device_streaming_t* s, const unsigned char* str)
{
    if(s->device->type == EDVS_SERIAL_DEVICE) {
        size_t pos = 0;
        while(str[pos] != 0) {
            unsigned char c;
            ssize_t n = edvs_device_read(s->device, &c, 1);
            if(n != 1) {
                printf("ERROR wait_for\n");
                return -1;
            }
            if(c == str[pos]) {
                pos ++;
            }
            else {
                pos = 0;
            }
        }
        return 0;
    }
    if(s->device->type == EDVS_NETWORK_DEVICE) {
        // FIXME
        printf("ERROR flush for network device not implemented\n");
        return -1;
    }
    printf("edvs_run: unknown stream type\n");
    return -1;
}

// called by dvs128_run to collect events from dvs
int edvs_device_streaming_run(edvs_device_streaming_t* s)
{
    s->systime_offset = get_micro_time();
    if(s->master_slave_mode == 0) {
#ifdef EDVS_LOG_MESSAGE
        printf("Running as normal (no master/slave)\n");
#endif
    }
    else if(s->master_slave_mode == 1) {
#ifdef EDVS_LOG_MESSAGE
        printf("Running as master\n");
#endif
        // master is giving the go command
        if(edvs_device_write_str(s->device, "!ETM+\n") != 0)
            return -1;          
    }
    else if(s->master_slave_mode == 2) {
#ifdef EDVS_LOG_MESSAGE
        printf("Running as slave\n");
#endif
    }
    else {
        printf("ERROR in edvs_device_streaming_run: Invalid master/slave mode!\n");
    }
    // starting event transmission
#ifdef EDVS_LOG_MESSAGE
    printf("Starting transmission\n");
#endif
    if(edvs_device_write_str(s->device, "E+\n") != 0)
        return -1;
    // wait until we get E+\n back
    wait_for(s, "E+\n");
    return 0;
}

/** Computes delta time between timestamps considering a possible wrap */
uint64_t timestamp_dt(uint64_t t1, uint64_t t2, uint64_t wrap)
{
    if(t2 >= t1) {
        // no wrap
        return t2 - t1;
    }
    else {
        // wrap (assume 1 wrap)
        return (wrap + t2) - t1;
    }
}

/** Unwraps timestamps the easy way (works good for 3 byte ts, be careful with 2 byte ts) */
void compute_timestamps_incremental(edvs_event_t* begin, size_t n, uint64_t last_device, uint64_t last_host, uint64_t wrap)
{
    edvs_event_t* end = begin + n;
    for(edvs_event_t* events=begin; events!=end; ++events) {
        // current device time (wrapped)
        uint64_t t = events->t;
        // delta time since last
        uint64_t dt = timestamp_dt(last_device, t, wrap);
#ifdef EDVS_LOG_VERBOSE
        if(t < last_device) {
            printf("WRAPPING %zd -> %zd\n", last_device, t);
        }
#endif
        // update timestamp
        last_device = t;
        // update timestamp
        last_host += dt;
        events->t = last_host;
//      printf("%"PRIu64"\t%"PRIu64"\n", t, events->t);
    }
}

uint64_t sum_dt(edvs_event_t* begin, edvs_event_t* end, uint64_t last_device, uint64_t wrap)
{
    uint64_t dtsum_device = 0;
    for(edvs_event_t* events=begin; events!=end; ++events) {
        // current device time (wrapped)
        uint64_t t = events->t;
        // delta time since last
        uint64_t dt = timestamp_dt(last_device, t, wrap);
        // update timestamp
        last_device = t;
        // sum up
        dtsum_device += dt;
    }
    return dtsum_device;
}

/** Unwraps timestamps using some ugly black magic */
void compute_timestamps_systime(edvs_event_t* begin, size_t n, uint64_t last_device, uint64_t last_host, uint64_t wrap, uint64_t systime)
{
    edvs_event_t* end = begin + n;
    // compute the total added delta time for all events (device time)
    uint64_t dtsum_device = sum_dt(begin, end, last_device, wrap);
    // delta time for all events (host time)
    uint64_t dtsum_host = systime - last_host;
//  printf("SUM DEVICE %"PRIu64"\tSUM HOST %"PRIu64"\n", dtsum_device, dtsum_host);
    // problem: delta time for device might differ from delta time for host
    // if dt_D < dt_H: this might have a natural cause, so we do not know if we need to do something
    // if dt_D > dt_H: we have a problem as timestamps would not be ordered. to solve this we scale timestamps
    uint64_t rem = 0;
//  printf("%"PRIu64"\t%"PRIu64"\n", (end - 1)->t, systime);
    uint64_t curr_host = systime;
    uint64_t next_set_host = systime;
    for(edvs_event_t* events=end-2; events>=begin; --events) { // iterate backwards (skip last)
        // current device time (wrapped)
        uint64_t t = events->t;
        // current delta time
        uint64_t dt_device = timestamp_dt(t, (events+1)->t, wrap);
        // rescale
        // rem_i-1 + dtD*sH = dtH*sD + rem_i, 0 <= rem_i < dtH
        uint64_t s = rem + dt_device*dtsum_host;
        rem = s % dtsum_device;
        uint64_t dt_host = (s - rem) / dtsum_device;
        curr_host -= dt_host;
        (events+1)->t = next_set_host; // delay set to not corrupt device timesteps needed for computation
        next_set_host = curr_host;
//      printf("AAA %"PRIu64"\t%"PRIu64"\t%"PRIu64"\t%"PRIu64"\n", dt_device, dt_host, s, rem);
//      printf("%"PRIu64"\t%"PRIu64"\n", t, curr_host);
    }
    begin->t = next_set_host; // set last here due to delayed set
}

// the most important function of all
// ssize_t edvs_device_streaming_read(edvs_device_streaming_t* s, edvs_event_t* events, size_t n, edvs_special_t* special, size_t* ns)
ssize_t dvs128_device_streaming_read(dvs128_device_streaming_t* s, dvs128_event_t* events, size_t n, dvs128_special_t* special, size_t* ns)
{
    // parse events
    ssize_t i = 0; // index of current byte
    edvs_event_t* event_it = events;
    edvs_special_t* special_it = special;
#ifdef EDVS_LOG_ULTRA
    printf("START\n");
#endif
    while(i+cNumBytesAhead < bytes_read) {
        // break if no more room for special
        if(special != 0 && ns != 0 && num_special >= *ns) {
            break;
        }
        // get two bytes
        unsigned char a = buffer[i];
        unsigned char b = buffer[i + 1];
#ifdef EDVS_LOG_ULTRA
//      printf("e: %d %d\n", a, b);
#endif
        i += 2;
        // check for and parse 1yyyyyyy pxxxxxxx
        if((a & cHighBitMask) == 0) { // check that the high bit of first byte is 1
            // the serial port missed a byte somewhere ...
            // skip one byte to jump to the next event
            printf("Error in high bit! Skipping a byte\n");
            i --;
            continue;
        }
        // check for special data
        size_t special_data_len = 0;
        if(special != 0 && a == 0 && b == 0) {
            // get special data length
            special_data_len = (buffer[i] & 0x0F);
            // HACK assuming special data always sends timestamp!
            if(special_data_len >= cNumBytesTimestamp) {
                special_data_len -= cNumBytesTimestamp;
            }
            else {
                printf("ERROR parsing special data length!\n");
            }
            i ++;
#ifdef EDVS_LOG_ULTRA
//          printf("s: len=%ld\n", special_data_len);
#endif
        }
        // read timestamp
        uint64_t timestamp;
        if(timestamp_mode == 1) {
            timestamp =
                  ((uint64_t)(buffer[i  ]) <<  8)
                |  (uint64_t)(buffer[i+1]);
        }
        else if(timestamp_mode == 2) {
            timestamp =
                  ((uint64_t)(buffer[i  ]) << 16)
                | ((uint64_t)(buffer[i+1]) <<  8)
                |  (uint64_t)(buffer[i+2]);
            // printf("%d %d %d %d %d\n", a, b, buffer[i+0], buffer[i+1], buffer[i+2]);
#ifdef EDVS_LOG_ULTRA
            printf("t: %d %d %d -> %ld\n", buffer[i], buffer[i+1], buffer[i+2], timestamp);
#endif
        }
        else if(timestamp_mode == 3) {
            timestamp =
                  ((uint64_t)(buffer[i  ]) << 24)
                | ((uint64_t)(buffer[i+1]) << 16)
                | ((uint64_t)(buffer[i+2]) <<  8)
                |  (uint64_t)(buffer[i+3]);
        }
        else {
            timestamp = 0;
        }
//      printf("%p %zd\n", s, timestamp);
        // advance byte count
        i += cNumBytesTimestamp;

        if(special != 0 && ns != 0 && a == 0 && b == 0) {
            // create special
            special_it->t = timestamp; // FIXME s->current_time;
            special_it->n = special_data_len;
            // read special data
#ifdef EDVS_LOG_ULTRA
            printf("SPECIAL DATA:");
#endif
            for(size_t k=0; k<special_it->n; k++) {
                special_it->data[k] = buffer[i+k];
#ifdef EDVS_LOG_ULTRA
                printf(" %d", special_it->data[k]);
#endif
            }
#ifdef EDVS_LOG_ULTRA
            printf("\n");
#endif
            i += special_it->n;
            special_it++;
            num_special++;
        }
        else {
            // create event
            event_it->t = timestamp;
            event_it->x = (uint16_t)(b & cLowerBitsMask);
            event_it->y = (uint16_t)(a & cLowerBitsMask);
            event_it->parity = ((b & cHighBitMask) ? 1 : 0);
            event_it->id = 0;
            event_it++;
        }
    }
    // i is now the number of processed bytes
    s->offset = bytes_read - i;
    if(s->offset > 0) {
        for(size_t j=0; j<s->offset; j++) {
            buffer[j] = buffer[i + j];
        }
    }
    // return
    if(ns != 0) {
        if(special != 0) {
            *ns = special_it - special;
        }
        else {
            *ns = 0;
        }
    }
    // number of events
    ssize_t num_events = event_it - events;
#ifdef EDVS_LOG_ULTRA
    printf("Parsed %zd events\n", num_events);
#endif
    // correct timestamps
    if(num_events > 0) {
        uint64_t last_device = s->ts_last_device;
        uint64_t last_host = s->ts_last_host;
        if(s->ts_last_host == c_uint64_t_max) {
            last_device = events->t;
            last_host = 0;
        }
//      printf("LAST DEVICE %"PRIu64"\n", last_device);
//      printf("LAST HOST %"PRIu64"\n", last_host);
        s->ts_last_device = (events + num_events - 1)->t;
        if(s->host_timestamp_mode == 1) {
            compute_timestamps_incremental(events, num_events, last_device, last_host, cTimestampLimit);
        }
        if(s->host_timestamp_mode == 2) {
            compute_timestamps_systime(events, num_events, last_device, last_host, cTimestampLimit, system_clock_time - s->systime_offset);
        }
        s->ts_last_host = (events + num_events - 1)->t;
    }
    return num_events;
}

int dvs128_device_streaming_write(edvs_device_streaming_t* s, const char* cmd, size_t n)
{
    if(edvs_device_write(s->device, cmd, n) != n)
        return -1;
    return 0;
}

int edvs_device_streaming_stop(edvs_device_streaming_t* s)
{
    int r = dvs128_device_streaming_write(s, "E-\n", 3);
    if(r != 0) return r;
    free(s->buffer);
    free(s);
    return 0;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <stdint.h>
#include <string.h>

// functions below are used in EvenStream.cpp



dvs128_stream_handle dvs128_open()
{

    caerDeviceHandle dvs128_handle = caerDeviceOpen(1, CAER_DEVICE_DVS128, 0, 0, NULL);
    if (dvs128_handle == NULL) {
        printf("dvs128_open: unable to open dvs128")
        return (EXIT_FAILURE);
    }
    return caerDeviceOpen(1, CAER_DEVICE_DVS128, 0, 0, NULL);
}


int dvs128_run(dvs128_stream_handle s)
{
    dvs128_device_streaming_t* ds = (dvs128_device_streaming_t*)s->handle;
    return dvs128_device_streaming_run(ds);
}

int dvs128_close(dvs128_stream_handle s)
{
    dvs128_device_streaming_t* ds = (dvs128_device_streaming_t*)s->handle;
    dvs128_device_t* dh = ds->device;
    dvs128_device_streaming_stop(ds);
    dvs128_device_close(dh);
    free(s);
    return 0;
}

int dvs128_is_live(dvs128_stream_handle s)
{
    // first try streaming from the device directly
    return 1; // this is a huge hack
    /*
    if(s->type == EDVS_DEVICE_STREAM) {
        return 1;
    }
    if(s->type == EDVS_FILE_STREAM) {
        return 0;
    }
    printf("edvs_is_live: unknown stream type\n");
    return -1;
    */
}

int dvs128_eos(dvs128_stream_handle s)
{
    return 0; // devices continuously keep giving events
    /*
    if(s->type == EDVS_DEVICE_STREAM) {
        // TODO can device streams reach end of stream?
        return 0;
    }
    if(s->type == EDVS_FILE_STREAM) {
        edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
        return ds->is_eof;
    }
    printf("edvs_eos: unknown stream type\n");
    return -1;
    */
}

int dvs128_get_master_slave_mode(dvs128_stream_handle s)
{
    dvs128_device_streaming_t* ds = (dvs128_device_streaming_t*)s->handle;
    return ds->master_slave_mode;
    /*
    if(s->type == EDVS_DEVICE_STREAM) {
        edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
        return ds->master_slave_mode;
    }
    if(s->type == EDVS_FILE_STREAM) {
        return 0;
    }
    printf("edvs_get_master_slave_mode: unknown stream type\n");
    return -1;
    */
}

ssize_t dvs128_read(dvs128_stream_handle s, dvs128_event_t* events, size_t n)
{
    return dvs128_read_ext(s, events, n, 0, 0);
}

ssize_t dvs128_read_ext(dvs128_stream_handle s, dvs128_event_t* events, size_t n, dvs128_special_t* special, size_t* ns)
{
    dvs128_device_streaming_t* ds = (dvs128_device_streaming_t*)s->handle;
    return dvs128_device_streaming_read(ds, events, n, special, ns);
    /*
    if(s->type == EDVS_DEVICE_STREAM) {
        edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
        return edvs_device_streaming_read(ds, events, n, special, ns);
    }
    if(s->type == EDVS_FILE_STREAM) {
        edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
        if(ns != 0) {
            *ns = 0;
        }
        return edvs_file_streaming_read(ds, events, n);
    }
    printf("edvs_read: unknown stream type\n");
    return -1;
    */
}

ssize_t dvs128_write(dvs128_stream_handle s, const char* cmd, size_t n)
{
    dvs128_device_streaming_t* ds = (dvs128_device_streaming_t*)s->handle;
    return dvs128_device_streaming_write(ds, cmd, n);
    /**
    if(s->type == EDVS_DEVICE_STREAM) {
        edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
        return edvs_device_streaming_write(ds, cmd, n);
    }
    if(s->type == EDVS_FILE_STREAM) {
        printf("edvs_write: ERROR can not write to file stream!\n");
        return -1;
    }
    printf("edvs_write: unknown stream type\n");
    return -1;
    */
}
