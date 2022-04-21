#pragma once

#include "../utils.h"
#include "plugin.h"
#include "../debugger/debugger.h"

class DebuggerControlPlugin: public IPlugin {
public:
    DebuggerControlPlugin() = default;
    virtual void update() override;
    virtual void show() override;
    virtual void destroy() override;

    void runExecution();
    void continueExecution();
    void stopExecution();

    void stepOver();
    void stepInto();
    void stepOut();

    void setBreakpoint(const std::string &file, int line);
    void removeBreakpoint(const std::string &file, int line);
    void removeAllBreakpoints();
    void setBreakpointEnabled(const std::string &file, int line, bool enabled);
    void setBreakpointCondition(const std::string &file, int line, const std::string &condition);
    
protected:

};