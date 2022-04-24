#pragma once

#include "../utils.h"

static std::unordered_set<std::string> wt_keywords = {
    "#mode", "#include", "once", "input", "output", "int",   "float", "char",   "void",
    "type",  "if",       "else", "for",   "do",     "while", "pardo", "return",
};

static std::unordered_set<std::string> wt_identifiers = {
    "sqrt", "sqrtf", "log", "logf", "sort", "size", "dim", "EREW", "CREW", "cCRCW",
};

// @ breakpoint