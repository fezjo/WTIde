#pragma once

#include "../utils.h"
#include "plugin.h"

#include "../debugger/debugger.h" // TODO why does it need to be at the end

class ProgramAnalyzerPlugin : public IPlugin {
public:
    ProgramAnalyzerPlugin(Debugger *debugger);
    void show() override;

    void refresh();
protected:
    Debugger *debugger;
    Breakpoint edit_bp;
};