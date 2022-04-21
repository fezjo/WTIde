#pragma once

#include "../utils.h"

namespace WTStar
{
    extern "C"
    {
        #include "vm.h"
    }
}

struct Breakpoint {
    std::string file;
    int line;
    bool enabled;
    std::string condition;
};

class Debugger {
protected:
    std::vector<Breakpoint> breakpoints;
};