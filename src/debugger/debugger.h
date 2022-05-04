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
void error_handler_callback(WTStar::error_t *error, void *data);
}

class Debugger {
public:
    Debugger();
    ~Debugger();

    bool setSource(const std::string &source_fn);
    bool compile(bool memory = false);
    bool initialize(bool memory = false);

    void setInput(const std::string &input);
    std::string getOutput() const;
    std::string getCompilationOutput() const;
    void clearCompilationOutput();

    std::string getSource() const;
    std::pair<size_t, SourcePosition> getSourcePosition() const;
    bool isCompiled() const;
    bool isRunning() const;
    bool canRun() const;

    int runExecution();
    int continueExecution();
    void pauseExecution();
    void stopExecution();

    int stepOver();
    int stepInto();
    int stepOut();

    std::pair<bool, std::string> setBreakpoint(const std::string &file, uint line);
    std::pair<bool, std::string> setBreakpointWithCondition(const std::string &file, uint line,
                                                            const std::string &condition);
    bool setBreakpointEnabled(const std::string &file, uint line, bool enabled);
    bool removeBreakpoint(const std::string &file, uint line);
    void removeAllBreakpoints();

protected:
    void destroy();
    void destroyVm();
    bool readBinary();
    bool readInput();

    uint findInstructionNumber(const std::string &file, uint line);
    Breakpoint *findBreakpoint(const std::string &file, uint line);
    bool addBreakpointToVm(Breakpoint &bp);

public:
    bool trace_on = false;
    bool stop_on_bp = true;

protected:
    std::string source_fn = "";
    std::string binary_fn = "";
    std::string input = "";
    std::vector<Breakpoint> breakpoints = {};

    ErrorHandler error_handler;

    bool compiled = false;
    WTStar::include_project_t *ip = nullptr;
    WTStar::ast_t *ast = nullptr;
    WTStar::virtual_machine_t *env = nullptr;
    code_t binary = {};

    friend class ProgramAnalyzerPlugin;
    friend class DebuggerVariableViewerPlugin;
};