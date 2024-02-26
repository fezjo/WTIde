#pragma once

#include "debugger_control_plugin.h"

class DebuggerControlPluginV1 : public DebuggerControlPlugin {
public:
    DebuggerControlPluginV1(Debugger* debugger) : DebuggerControlPlugin(debugger) {}
    void show() override;
    DebuggerActionResult setSourceAction() override;
    DebuggerActionResult setSourceAction(const std::string& source) override;

protected:
    std::string file_buffer;
    ImVec4 source_fn_color = ImVec4();
};