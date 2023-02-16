#pragma once

#include "../utils.h"
#include "breakpoint.h"
#include "debugger.h"

class Debugger;

class BreakpointManager {
public:
    Debugger *debugger;
    std::set<Breakpoint> bp_working_set;
    std::vector<BreakpointCallbacks> handlers;

public:
    BreakpointManager() = default;
    BreakpointManager(Debugger *debugger) : debugger(debugger) {};
    ~BreakpointManager() = default;

    std::vector<Breakpoint> getBreakpoints() const;
    int contains(const Breakpoint& bp) const;
    void addHandler(const BreakpointCallbacks& handler);
    bool setBreakpoint(const Breakpoint& bp);
    bool removeBreakpoint(const Breakpoint& bp);

protected:
    bool _createBreakpoint(const Breakpoint& bp);
    bool _updateBreakpoint(const Breakpoint& bp);
    bool _removeBreakpoint(const Breakpoint& bp);
};