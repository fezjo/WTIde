#pragma once 

#include "../utils.h"

struct Breakpoint {
    std::string file;
    int line;
    bool enabled;
    std::string condition;

    std::weak_ordering operator<=>(const Breakpoint& other) const {
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
       << bp.condition << "')";
    return os;
}

using bp_change_callback_t = std::function<bool(const Breakpoint&)>;
using bp_gather_callback_t = std::function<std::set<Breakpoint>()>;

struct BreakpointCallbacks {
    bp_change_callback_t update, remove;
    bp_gather_callback_t gather;
    std::string info;
    size_t id;

    BreakpointCallbacks() : info("emptydefault"), id(0) {};
    BreakpointCallbacks(bp_change_callback_t update, bp_change_callback_t remove,
                        bp_gather_callback_t gather)
        : update(update), remove(remove), gather(gather), info("noinfo"), id(generateId()) {}
    BreakpointCallbacks(bp_change_callback_t update, bp_change_callback_t remove,
                        bp_gather_callback_t gather, std::string info)
        : update(update), remove(remove), gather(gather), info(info), id(generateId()) {}
    static size_t generateId() { return _ID++; }

private:
    inline static size_t _ID = 1;
};