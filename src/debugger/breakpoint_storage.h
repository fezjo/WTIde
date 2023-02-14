#pragma once

#include "../utils.h"

struct Breakpoint {
    std::string file;
    int line;
    bool enabled;
    std::string condition;

    auto operator<=>(const Breakpoint& other) const {
        if (auto cmp = file <=> other.file; cmp != 0)
            return cmp;
        return line <=> other.line;
    }

    bool identical(const Breakpoint& other) const {
        return file == other.file && line == other.line && enabled == other.enabled &&
               condition == other.condition;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Breakpoint& bp) {
    os << "Breakpoint(" << bp.file << ":" << bp.line << ", " << bp.enabled << ", cond='"
       << bp.condition << ")";
    return os;
}

using bp_callback_t = std::function<void(const Breakpoint&)>;

struct BreakpointHandler {
    bp_callback_t update;
    bp_callback_t remove;
    std::string info;
};

class BreakpointStorage {
public:
    std::set<Breakpoint> breakpoints;
    std::set<Breakpoint> bp_working_set;
    std::vector<BreakpointHandler> handlers;

public:
    BreakpointStorage() = default;
    ~BreakpointStorage() = default;

    void updateBreakpoint(const Breakpoint& bp) {
        if (bp_working_set.count(bp))
            return;
        bp_working_set.insert(bp);
        dbg("bpStorage.updateBreakpoint", handlers.size(), breakpoints.size(), bp);
        auto it = breakpoints.find(bp);
        if (it == breakpoints.end() || !it->identical(bp)) {
            breakpoints.insert(bp);
            for (auto& handler : handlers) {
                dbg("bpStorage.updateBreakpoint", handler.info);
                if (handler.update) {
                    dbg("bpStorage.updateBreakpoint called", handler.info);
                    handler.update(bp);
                }
            }
        } else
            dbg("bpStorage.updateBreakpoint identical", bp, *it);
        dbg("update done");
        bp_working_set.erase(bp);
    }

    void removeBreakpoint(const Breakpoint& bp) {
        if (bp_working_set.count(bp))
            return;
        bp_working_set.insert(bp);
        dbg("bpStorage.removeBreakpoint", bp);
        auto it = breakpoints.find(bp);
        if (it != breakpoints.end()) {
            breakpoints.erase(it);
            for (auto& handler : handlers)
                if (handler.remove)
                    handler.remove(bp);
        }
        dbg("remove done");
        bp_working_set.erase(bp);
    }

    void addHandler(const BreakpointHandler& handler) {
        dbg("bpStorage.addHandler");
        handlers.push_back(handler);
    }
};