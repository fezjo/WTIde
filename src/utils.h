#pragma once

#include <vector>
#include <fstream>
#include <iostream>
#include <string>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <functional>
#include <tuple>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <list>

namespace fs = std::filesystem;

using imid_t = uint64_t;

extern std::hash<std::string> hash_string;