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
            h = dvs128_open();
        }

        void close()
        {
            if(is_open()) {
                if(is_running_) {
                    is_running_ = false;
                    thread_.join();
                }
                if(h) {
                    dvs128_close(h);
                }
            }
        }

        void run()
        {
            last_time_ = 0;
            dvs128_run(h);
            is_running_ = true;
            thread_ = std::thread(&SingleEventStream::runImpl, this);
        }

        bool is_open() const
        {
            return h;
        }

        bool is_master() const
        {
            return dvs128_get_master_slave_mode(h) == 1;
        }

        bool is_slave() const
        {
            return dvs128_get_master_slave_mode(h) != 1;
        }

        bool eos() const
        {
            std::lock_guard<std::mutex> lock(mtx_);
            return is_open() && dvs128_eos(h) && events_.empty();
        }

        bool is_live() const
        {
            return dvs128_is_live(h) == 1;
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
                return v;
            }
        }

        uint64_t last_timestamp() const
        { return last_time_; }

        void write(const std::string& cmd) const
        {
            std::string cmdn = cmd;
            cmdn += '\n';
            dvs128_write(h, (char*)cmdn.data(), cmdn.length());
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
        }

    private:
        bool is_running_;
        std::thread thread_;
        mutable std::mutex mtx_;
        std::vector<dvs128_event_t> events_;
        dvs128_stream_handle h;
        uint64_t last_time_;
    };


    std::shared_ptr<IEventStream> OpenEventStream(const std::string& uri)
    {
        return std::make_shared<SingleEventStream>(uri);
    }

}
