#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "../../debugger/debugger.h" // TODO why does it need to be at the end

class ProgramAnalyzerPlugin : public IPlugin {
public:
    ProgramAnalyzerPlugin(Debugger* debugger);
    void show() override;

    void refresh();

protected:
    void showBreakpoints(WTStar::virtual_machine_t* env);

public:
    BreakpointCallbacks bp_callback;

protected:
    Debugger* debugger;
    VM_Breakpoint edit_bp{};
};