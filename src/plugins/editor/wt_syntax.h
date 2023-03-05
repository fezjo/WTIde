#pragma once

#include "../../utils.h"

#include "TextEditor.h"

static std::vector<std::string> wt_keywords = {
    "#mode", "#include", "once", "input", "output", "int",   "float", "char",   "void",
    "type",  "if",       "else", "for",   "do",     "while", "pardo", "return", "BRK",
};

static std::vector<std::string> wt_identifiers = {
    "sqrt", "sqrtf", "log", "logf", "sort", "size", "dim", "EREW", "CREW", "cCRCW",
};

static std::vector<std::string> wt_identifiers_help = {
    "ceiling(sqrt(x))",
    "sqrt(x)",
    "ceiling(log2(x))",
    "log2(x)",
    "Sort an array [T=O(log n), W=O(n log n)]",
    "Size",
    "Number of dimensions",
    "Exclusive Read Exclusive Write",
    "Concurrent Read Exclusive Write",
    "common Concurrent Read Concurrent Write",
};

const TextEditor::TextEditor::LanguageDefinition& TextEditor_LanguageDefinition_WTStar();