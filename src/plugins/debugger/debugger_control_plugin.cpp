#include "debugger_control_plugin.h"

DebuggerControlPlugin::DebuggerControlPlugin(Debugger *debugger) : debugger(debugger) {
    pluginType = PluginType::PluginControl;
}

bool DebuggerControlPlugin::setSource(const std::string &source) {
    if (!debugger->setSource(source))
        return false;
    source_fn = source;
    return true;
}

void DebuggerControlPlugin::setInput(const std::string &input) { debugger->setInput(input); }

bool DebuggerControlPlugin::setSourceAction() {
    auto path = std::get<std::string>(callbacks["get_focused_source"](0));
    return setSourceAction(path);
}

bool DebuggerControlPlugin::setSourceAction(const std::string &source) {
    if (!setSource(source)) {
        std::cerr << "Failed to set source" << std::endl;
        return false;
    }
    callbacks["refresh_analyzer"](0);
    if (!debugger->compile(true)) {
        std::cerr << "Failed to compile source" << std::endl;
        return false;
    }
    if (!debugger->initialize(true)) {
        std::cerr << "Failed to initialize" << std::endl;
        return false;
    }
    return true;
}

bool DebuggerControlPlugin::compileAction() {
    debugger->clearCompilationOutput();
    bool resp = debugger->compile();
    callbacks["refresh_analyzer"](0);
    callbacks["set_compilation_output"](debugger->getCompilationOutput());
    if (resp)
        debugger->initialize();
    if (resp)
        ImGui::InsertNotification({ImGuiToastType_Success, 5000, "Compilation successful"});
    else
        ImGui::InsertNotification({ImGuiToastType_Error, 5000, "Compilation failed"});
    return resp;
}

int DebuggerControlPlugin::runAction() {
    callbacks["set_input"](0);
    if (!debugger->isCompiled())
        if (!compileAction())
            return false;
    return debugger->runExecution();
}