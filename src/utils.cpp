#include "utils.h"

std::hash<std::string> hash_string;

timepoint get_time()
{
    return std::chrono::steady_clock::now();
}