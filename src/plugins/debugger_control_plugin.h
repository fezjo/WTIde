#pragma once

#include "../utils.h"
#include "plugin.h"

#include "../debugger/debugger.h" // TODO why does it need to be at the end

class DebuggerControlPlugin : public IPlugin {
public:
    DebuggerControlPlugin(Debugger *debugger);
    void show() override;

    void setSource(const std::string &source);
    void setInput(const std::string &input);

    // void runExecution();
    // void continueExecution();
    // void stopExecution();

    // void stepOver();
    // void stepInto();
    // void stepOut();

    // void setBreakpoint(const std::string &file, int line);
    // void removeBreakpoint(const std::string &file, int line);
    // void removeAllBreakpoints();
    // void setBreakpointEnabled(const std::string &file, int line, bool enabled);
    // void setBreakpointWithCondition(const std::string &file, int line, const std::string
    // &condition);

protected:
    Debugger *debugger;
    std::string source_fn = "test.wt"; // TODO testing
};