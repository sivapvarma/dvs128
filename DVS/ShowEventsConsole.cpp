#include <iostream>
#include "EventStream.hpp"
#include "EventIO.hpp"

int main()
{
    auto stream = dvs128::OpenEventStream();
    std::cout << "Initialization done!\n";
    auto events = stream->read();
    while(!events.empty())
    {
        std::cout << "Inside while.\n";
        for(auto e: events)
            std::cout << e << '\n';
        std::cout << "end of for loop.\n";
        events = stream->read();
    }
    return 0;
}
