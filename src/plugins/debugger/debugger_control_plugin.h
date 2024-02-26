#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "../../debugger/debugger.h" // TODO why does it need to be at the end

enum class DebuggerActionResult {
    Success = 0,
    Fail = -1,
    FailedToSetSource = -2,
    FailedToCompile = -3,
    FailedToInitialize = -4,
    FailedToRun = -5,
};

class DebuggerControlPlugin : public IPlugin {
public:
    DebuggerControlPlugin(Debugger* debugger);
    virtual ~DebuggerControlPlugin() = default;

    virtual DebuggerActionResult setSource(const std::string& source);
    virtual void setInput(const std::string& input);
    virtual DebuggerActionResult setSourceAction();
    virtual DebuggerActionResult setSourceAction(const std::string& source);
    virtual DebuggerActionResult compileAction();
    virtual int runAction();

protected:
    Debugger* debugger;
    std::string source_fn = "test.wt"; // TODO testing
    bool seen = false;
};