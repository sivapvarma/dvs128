#ifndef INCLUDE_DVS128_EVENTSTREAM_HPP
#define INCLUDE_DVS128_EVENTSTREAM_HPP

#include "Event.hpp"
#include <vector>
#include <string>
#include <memory>
#include <initializer_list>

namespace dvs128
{

    // base class with all virtual functions
    // this header file defines the IEventStream class
    class IEventStream
    {
    public:
        virtual ~IEventStream() {}
        virtual bool is_open() const = 0;
        virtual bool eos() const = 0;
        virtual bool is_live() const = 0;
        virtual std::vector<dvs128_event_t> read() = 0;
    };

    std::shared_ptr<IEventStream> OpenEventStream();


}

#endif
