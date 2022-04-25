#pragma once

#include "../utils.h"

namespace WTStar {
extern "C" {
#include "ast.h"
#include "code_generation.h"
#include "driver.h"
#include "errors.h"
#include "vm.h"
}
} // namespace WTStar

struct Breakpoint {
    std::string file;
    int line;
    std::string condition;
    bool enabled;
    uint32_t bp_pos;
};

class Writer {
public:
    Writer();
    Writer(const std::string &fn, const std::string &mode);
    ~Writer();
    void clear();
    std::string read(size_t pos = 0, size_t len = -1);

    WTStar::writer_t *w;
};


using code_t = std::vector<uint8_t>;

code_t readCode(const std::string &fn);

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

    uint32_t findInstructionNumber(const std::string &file, int line);
    Breakpoint *findBreakpoint(const std::string &file, int line);
    std::vector<uint8_t> compileCondition(const std::string &condition);

    bool compile();
    int runExecution();
    int continueExecution();
    void pauseExecution();
    void stopExecution();

    int stepOver();
    int stepInto();
    int stepOut();

    bool setBreakpoint(const std::string &file, int line);
    bool removeBreakpoint(const std::string &file, int line);
    void removeAllBreakpoints();
    bool setBreakpointEnabled(const std::string &file, int line, bool enabled);
    bool setBreakpointWithCondition(const std::string &file, int line,
                                    const std::string &condition);

protected:
    void reset();
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