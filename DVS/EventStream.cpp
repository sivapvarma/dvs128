#include "EventStream.hpp"
#include "dvs128.h"
#include "Event.hpp"
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <limits>
#include <iostream>

// libcaer
#include <libcaer/libcaer.h>
#include <libcaer/devices/dvs128.h>

namespace dvs128
{
    class SingleEventStream : public IEventStream
    {
    public:
        // SingleEventStream() {}

        SingleEventStream(const SingleEventStream&) = delete;
        SingleEventStream& operator=(const SingleEventStream&) = delete;

        SingleEventStream()
        {
            open();
            if(is_open()) {
                run();
            }
        }

        ~SingleEventStream()
        {
            close();
        }

        // void open(const std::string& uri)
        void open()
        {
            // h = dvs128_open();
            dh = caerDeviceOpen(1, CAER_DEVICE_DVS128, 0, 0, NULL);
            // send the default configuration to device
            caerDeviceSendDefaultConfig(dh);
        }

        void close()
        {
            // send stop signal to device
            caerDeviceDataStop(dh);
            /*
            if(is_open()) {
                if(is_running_) {
                    is_running_ = false;
                    thread_.join();
                }
                if(h) {
                    dvs128_close(h);
                }
            }
            */
        }

        void run()
        {
            last_time_ = 0;
            // dvs128_run(h);
            caerDeviceDataStart(dh, NULL, NULL, NULL, NULL, NULL);
            is_running_ = true;
            // this thread is no in device data start = libcaer
            thread_ = std::thread(&SingleEventStream::runImpl, this);
        }

        bool is_open() const
        {
            return dh;
        }

        bool is_master() const
        {
            // return dvs128_get_master_slave_mode(h) == 1;
            return true;// big hack
        }

        bool is_slave() const
        {
            // return dvs128_get_master_slave_mode(h) != 1;
            return false;// hack
        }

        bool eos() const
        {
            std::lock_guard<std::mutex> lock(mtx_);
            // return is_open() && dvs128_eos(h) && events_.empty();
            return is_open() && events_.empty();
        }

        bool is_live() const
        {
            // return dvs128_is_live(h) == 1;
            return true; // hack
        }

        std::vector<dvs128_event_t> read()
        {
            if(!is_open()) {
                return {};
            }
            else {
                std::vector<dvs128_event_t> v = pop_events();
                if(!v.empty()) {
                    last_time_ = v.back().t;
                }
                std::cout << "events returned\n";
                return v;
            }
        }

        uint64_t last_timestamp() const
        { return last_time_; }

        void write(const std::string& cmd) const
        {
            //std::string cmdn = cmd;
            //cmdn += '\n';
            //dvs128_write(h, (char*)cmdn.data(), cmdn.length());
            // return void;
        }

    private:
        std::vector<dvs128_event_t> pop_events()
        {
            std::lock_guard<std::mutex> lock(mtx_);
            std::vector<dvs128_event_t> tmp = std::move(events_); // TODO is this move correct?
            events_ = {};
            return tmp;
        }

        void runImpl()
        {
            std::vector<dvs128_event_t> v;
            while(true)
            {
                v.resize(1024);
                caerEventPacketContainer packetContainer = caerDeviceDataGet(dh);
                if (packetContainer == NULL) {
                    continue; // Skip if nothing there.
                }

                int32_t packetNum = caerEventPacketContainerGetEventPacketsNumber(packetContainer);

                printf("\nGot event container with %d packets (allocated).\n", packetNum);

                for (int32_t i = 0; i < packetNum; i++)
                {
                    caerEventPacketHeader packetHeader = caerEventPacketContainerGetEventPacket(packetContainer, i);
                    if (packetHeader == NULL) {
                        printf("Packet %d is empty (not present).\n", i);
                        continue; // Skip if nothing there.
                    }

                    ssize_t m = caerEventPacketHeaderGetEventNumber(packetHeader);
                    v.resize(m);
                    printf("Packet %d of type %d -> size is %d.\n", i, caerEventPacketHeaderGetEventType(packetHeader), m);
                    // Packet 0 is always the special events packet for DVS128
                    // while packet is the polarity events packet.
                    // if (i == POLARITY_EVENT)
                    // {
                        caerPolarityEventPacket polarity = (caerPolarityEventPacket) packetHeader;
                        for(int j=0; j<m; j++)
                        {
                            // loop over events
                            caerPolarityEvent ev = caerPolarityEventPacketGetEvent(polarity, j);
                            v[j].t = caerPolarityEventGetTimestamp(ev);
                            v[j].x = caerPolarityEventGetX(ev);
                            v[j].y = caerPolarityEventGetY(ev);
                            v[j].parity = caerPolarityEventGetPolarity(ev);
                            v[j].id = 0; // hack
                        }

                    // }
                }

                caerEventPacketContainerFree(packetContainer);
                std::lock_guard<std::mutex> lock(mtx_);
                events_.insert(events_.end(), v.begin(), v.end());
            }
            /*
            while(is_running_ && !dvs128_eos(h)) {
                v.resize(1024);
                ssize_t m = dvs128_read_ext(h, v.data(), v.size(), 0, 0);
                if(m >= 0) {
                    v.resize(m);
                }
                else {
                    v.clear();
                }
                std::lock_guard<std::mutex> lock(mtx_);
                events_.insert(events_.end(), v.begin(), v.end());
            }
            */
        }

    private:
        bool is_running_;
        std::thread thread_;
        mutable std::mutex mtx_;
        std::vector<dvs128_event_t> events_;
        // dvs128_stream_handle h;
        caerDeviceHandle dh;
        uint64_t last_time_;
    };


    std::shared_ptr<IEventStream> OpenEventStream()
    {
        return std::make_shared<SingleEventStream>();
    }

}
