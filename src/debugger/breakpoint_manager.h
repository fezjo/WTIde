#pragma once

#include "../utils.h"
#include "breakpoint.h"
#include "debugger.h"

class Debugger;

class BreakpointManager {
public:
    Debugger* debugger;
    std::vector<BreakpointCallbacks> handlers;
    bool locked = false;

public:
    BreakpointManager() = default;
    BreakpointManager(Debugger* debugger) : debugger(debugger) {}
    ~BreakpointManager() = default;

    void addHandler(const BreakpointCallbacks& handler);
    size_t removeHandler(size_t id);
    void synchronizeHandlers();
    std::set<Breakpoint> getBreakpoints() const;
    int containsBreakpoint(const Breakpoint& bp) const;
    bool updateBreakpoint(const Breakpoint& bp);
    bool removeBreakpoint(const Breakpoint& bp);
};