#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "../../debugger/debugger.h" // TODO why does it need to be at the end

class DebuggerVariableViewerPlugin : public IPlugin {
public:
    DebuggerVariableViewerPlugin(Debugger *debugger);
    void show() override;

    void refresh();
protected:
    Debugger *debugger;
};