#pragma once

#include "../utils.h"

namespace WTStar {
extern "C" {
#include "ast.h"
#include "ast_debug_print.h"
#include "code_generation.h"
#include "debug.h"
#include "driver.h"
#include "errors.h"
#include "vm.h"
}
} // namespace WTStar

class Writer {
public:
    Writer();
    Writer(const std::string &fn, const std::string &mode);
    ~Writer();
    void clear();
    std::string read(size_t pos = 0, size_t len = -1u);

    WTStar::writer_t *w;
};

using code_t = std::vector<uint8_t>;

code_t readCode(const std::string &fn);

struct Breakpoint {
    std::string file;
    uint line;
    std::string condition;

    uint bp_pos;
    std::string error;
    code_t code;
    WTStar::breakpoint_t *vm_bp;

    bool enabled;
};

struct SourcePosition {
    SourcePosition() = default;
    SourcePosition(WTStar::item_info_t *info, WTStar::virtual_machine_t *env);

    uint32_t fileid, //!< index of the source file
        fl,          //!< first line
        fc,          //!< first column
        ll,          //!< last line
        lc;          //!< last column
    std::string file;
    size_t line;
    bool valid = false;
};

SourcePosition findSourcePosition(WTStar::virtual_machine_t *env, int instruction_number);

extern "C" {
void debugger_error_handler(WTStar::error_t *error, void *data);
}

class Debugger {
public:
    Debugger() = default;
    ~Debugger();

    bool setSource(const std::string &source_fn);
    void setInput(const std::string &input);
    std::string getOutput();
    std::string getCompilationOutput();
    void clearCompilationOutput();

    uint findInstructionNumber(const std::string &file, uint line);
    std::pair<size_t, SourcePosition> getSourcePosition();
    Breakpoint *findBreakpoint(const std::string &file, uint line);
    std::vector<uint8_t> compileCondition(const std::string &condition);

    bool compile();
    int runExecution();
    int continueExecution();
    void pauseExecution();
    void stopExecution();

    int stepOver();
    int stepInto();
    int stepOut();

    bool setBreakpoint(const std::string &file, uint line);
    bool setBreakpointEnabled(const std::string &file, uint line, bool enabled);
    bool setBreakpointWithCondition(const std::string &file, uint line,
                                    const std::string &condition);
    bool removeBreakpoint(const std::string &file, uint line);
    void removeAllBreakpoints();

protected:
    void reset();
    void resetVm();
    bool readBinary();
    bool readInput();
    bool initialize();

    void errorHandler(WTStar::error_t *error);
    friend void debugger_error_handler(WTStar::error_t *error, void *data);

public:
    bool stop_on_bp = true;

protected:
    std::string source_fn = "";
    std::string binary_fn = "";
    code_t binary = {};
    std::string input = "";
    std::stringstream error_stream;

    WTStar::virtual_machine_t *env = nullptr;
    WTStar::ast_t *ast = nullptr;
    WTStar::include_project_t *ip = nullptr;
    std::vector<Breakpoint> breakpoints = {};

    friend class ProgramAnalyzerPlugin;
};