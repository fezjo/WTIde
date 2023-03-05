#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "../../debugger/debugger.h" // TODO why does it need to be at the end

class DebuggerControlPlugin : public IPlugin {
public:
    DebuggerControlPlugin(Debugger* debugger);
    virtual ~DebuggerControlPlugin() = default;
    virtual void show() = 0;

    virtual bool setSource(const std::string& source);
    virtual void setInput(const std::string& input);
    virtual bool setSourceAction();
    virtual bool setSourceAction(const std::string& source);
    virtual bool compileAction();
    virtual int runAction();

protected:
    Debugger* debugger;
    std::string source_fn = "test.wt"; // TODO testing
    bool seen = false;
};