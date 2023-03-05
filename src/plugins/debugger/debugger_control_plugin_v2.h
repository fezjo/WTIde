#pragma once

#include "debugger_control_plugin.h"

class DebuggerControlPluginV2 : public DebuggerControlPlugin {
public:
    DebuggerControlPluginV2(Debugger* debugger) : DebuggerControlPlugin(debugger){};
    void show() override;
};