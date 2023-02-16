#include "breakpoint_manager.h"

std::vector<Breakpoint> BreakpointManager::getBreakpoints() const {
    if (debugger == nullptr)
        return {};
    std::vector<Breakpoint> bps;
    for (auto& bp : debugger->breakpoints)
        bps.push_back(bp);
    return bps;
}

int BreakpointManager::contains(const Breakpoint& bp) const {
    auto breakpoints = getBreakpoints();
    auto it = std::find(breakpoints.begin(), breakpoints.end(), bp);
    if (it == breakpoints.end())
        return 0;
    if (bp.identical(*it))
        return 2;
    return 1;
}

void BreakpointManager::addHandler(const BreakpointCallbacks& handler) {
    dbg("bpStorage.addHandler");
    handlers.push_back(handler);
}

bool BreakpointManager::setBreakpoint(const Breakpoint& bp) {
    bool res = false;
    if (bp_working_set.count(bp))
        return res;
    bp_working_set.insert(bp);
    auto cont = contains(bp);
    if (cont == 1) res = _updateBreakpoint(bp);
    else if (cont == 0) res = _createBreakpoint(bp);
    bp_working_set.erase(bp);
    return res;
}

bool BreakpointManager::removeBreakpoint(const Breakpoint& bp) {
    bool res = false;
    if (bp_working_set.count(bp))
        return res;
    bp_working_set.insert(bp);
    if (contains(bp)) res = _removeBreakpoint(bp);
    bp_working_set.erase(bp);
    return res;
}

bool BreakpointManager::_createBreakpoint(const Breakpoint& bp) {
    dbg("bpStorage.createBreakpoint", handlers.size(), debugger->breakpoints.size(), bp);
    debugger->setBreakpoint(bp.file, bp.line, bp.enabled, bp.condition);
    for (auto& handler : handlers)
        if (handler.create) {
            dbg("bpStorage.createBreakpoint called", handler.info);
            handler.create(bp);
        }
    dbg("create done");
}

bool BreakpointManager::_updateBreakpoint(const Breakpoint& bp) {
    dbg("bpStorage.updateBreakpoint", handlers.size(), debugger->breakpoints.size(), bp);
    debugger->setBreakpoint(bp.file, bp.line, bp.enabled, bp.condition);
    for (auto& handler : handlers)
        if (handler.update) {
            dbg("bpStorage.updateBreakpoint called", handler.info);
            handler.update(bp);
        }
    dbg("update done");
    return true;
}

bool BreakpointManager::_removeBreakpoint(const Breakpoint& bp) {
    dbg("bpStorage.removeBreakpoint", bp);
    if (!debugger->removeBreakpoint(bp.file, bp.line))
        return false;
    for (auto& handler : handlers)
        if (handler.remove) {
            dbg("bpStorage.removeBreakpoint called", handler.info);
            handler.remove(bp);
        }
    dbg("remove done");
    return true;
}