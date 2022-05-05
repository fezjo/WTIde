#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "../../debugger/debugger.h" // TODO why does it need to be at the end

class DebuggerControlPlugin : public IPlugin {
public:
    DebuggerControlPlugin(Debugger *debugger);
    void show() override;

    bool setSource(const std::string &source);
    void setInput(const std::string &input);
    bool setSourceAction(const std::string &source);
    bool compileAction();
    int runAction();

protected:
    Debugger *debugger;
    std::string source_fn = "test.wt"; // TODO testing
    bool seen = false;
    ImVec4 source_fn_color = ImVec4();
};