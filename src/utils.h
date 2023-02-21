#pragma once

#include <algorithm>
#include <assert.h>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#include "dbg.h"

namespace fs = std::filesystem;

using imid_t = uint64_t;

extern std::hash<std::string> hash_string;

using timepoint = std::chrono::time_point<std::chrono::steady_clock>;
timepoint get_time();

bool ends_with(const std::string &str, const std::string &suffix);

fs::path normalize_path(const fs::path &path);