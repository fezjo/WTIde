#include "breakpoint_manager.h"

void BreakpointManager::addHandler(const BreakpointCallbacks& handler) {
    dbg("bpStorage.addHandler");
    handlers.push_back(handler);
}

void BreakpointManager::synchronizeHandlers() {
    locked = true;
    auto breakpoints = getBreakpoints();
    for (auto &handler: handlers) {
        auto bp_add = breakpoints;
        decltype(bp_add) bp_remove;
        for (auto bp: handler.gather()) {
            auto mng_bp = bp_add.find(bp);
            if (mng_bp != bp_add.end() && mng_bp->identical(bp))
                bp_add.erase(mng_bp);
            else
                bp_remove.insert(bp);
        }
        for (auto bp: bp_remove)
            handler.remove(bp);
        for (auto bp: bp_add)
            handler.update(bp);
    }
    locked = false;
}

std::set<Breakpoint> BreakpointManager::getBreakpoints() const {
    if (debugger == nullptr)
        return {};
    std::set<Breakpoint> bps;
    for (auto& bp : debugger->breakpoints)
        bps.insert(bp);
    return bps;
}

int BreakpointManager::containsBreakpoint(const Breakpoint& bp) const {
    auto breakpoints = getBreakpoints();
    auto it = std::find(breakpoints.begin(), breakpoints.end(), bp);
    if (it == breakpoints.end())
        return 0;
    if (bp.identical(*it))
        return 2;
    return 1;
}

bool BreakpointManager::updateBreakpoint(const Breakpoint& bp) {
    if (containsBreakpoint(bp) != 2)
        debugger->setBreakpoint(bp.file, bp.line, bp.enabled, bp.condition);
    return true;
}

bool BreakpointManager::removeBreakpoint(const Breakpoint& bp) {
    if (containsBreakpoint(bp))
        return debugger->removeBreakpoint(bp.file, bp.line);
}
