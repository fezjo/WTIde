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

    bool operator==(const Breakpoint& other) const = default;

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

using bp_callback_t = std::function<bool(const Breakpoint&)>;

struct BreakpointCallbacks {
    bp_callback_t create, update, remove;
    std::string info;
};