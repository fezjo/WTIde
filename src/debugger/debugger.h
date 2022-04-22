#pragma once

#include "../utils.h"

namespace WTStar
{
    extern "C"
    {
        #include "vm.h"
        #include "ast.h"
        #include "driver.h"
        #include "code_generation.h"
    }
}

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
    std::string read(size_t pos=0, size_t len=-1);

    WTStar::writer_t *w;
};

using code_t = std::vector<uint8_t>;

class Debugger {
public:
    Debugger() = default;
    ~Debugger();

    bool setSource(const std::string &source_fn);
    void setInput(const std::string &input);
    std::string getOutput();

    uint32_t findInstructionNumber(const std::string &file, int line);
    Breakpoint* findBreakpoint(const std::string &file, int line);
    std::vector<uint8_t> compileCondition(const std::string &condition);

    int runExecution();
    int continueExecution();
    void pauseExecution();
    void stopExecution();

    void stepOver();
    void stepInto();
    void stepOut();

    bool setBreakpoint(const std::string &file, int line);
    bool removeBreakpoint(const std::string &file, int line);
    void removeAllBreakpoints();
    bool setBreakpointEnabled(const std::string &file, int line, bool enabled);
    bool setBreakpointWithCondition(const std::string &file, int line, const std::string &condition);

protected:
    void reset();
    bool readBinary();
    bool readInput();
    bool compile();
    bool initialize();

protected:
    std::string source_fn = "";
    std::string binary_fn = "";
    code_t binary = {};
    std::string input = "";
    
    WTStar::virtual_machine_t *env = nullptr;
    WTStar::ast_t *ast = nullptr;
    WTStar::include_project_t *ip = nullptr;
    std::vector<Breakpoint> breakpoints = {};
};