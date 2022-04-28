#pragma once

#include "../utils.h"
#include "wtstar_utils.h"

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

    std::pair<size_t, SourcePosition> getSourcePosition();

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
    void destroy();
    void destroyVm();
    bool readBinary();
    bool readInput();
    bool initialize();

    uint findInstructionNumber(const std::string &file, uint line);
    Breakpoint *findBreakpoint(const std::string &file, uint line);
    std::vector<uint8_t> compileCondition(const std::string &condition);

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