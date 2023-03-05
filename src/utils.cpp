#include "utils.h"

std::hash<std::string> hash_string;

timepoint get_time() { return std::chrono::steady_clock::now(); }

bool ends_with(const std::string &str, const std::string &suffix) {
    if (str.size() < suffix.size())
        return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

fs::path normalize_path(const fs::path &path) {
    if (path.empty())
        return path;
    return fs::relative(fs::canonical(path), fs::current_path());
}