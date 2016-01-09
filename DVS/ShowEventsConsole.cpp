#include <iostream>
#include "DVS128/EventStream.hpp"
#include "DVS128/EventIO.hpp"

int main()
{
    auto stream = dvs128::OpenEventStream();
    auto events = stream.read();
    while(!events.empty())
    {
        for(auto e: events)
            std::cout << e << '\n';
        std::cout << "end of for loop.\n"
        events = stream.read();
    }
    return 0;
}
