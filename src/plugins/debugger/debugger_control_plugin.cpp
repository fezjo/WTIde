#include "debugger_control_plugin.h"

DebuggerControlPlugin::DebuggerControlPlugin(Debugger* debugger) : debugger(debugger) {
    pluginType = PluginType::PluginControl;
}

DebuggerActionResult DebuggerControlPlugin::setSource(const std::string& source) {
    if (!debugger->setSource(source))
        return DebuggerActionResult::FailedToSetSource;
    source_fn = source;
    return DebuggerActionResult::Success;
}

void DebuggerControlPlugin::setInput(const std::string& input) { debugger->setInput(input); }

DebuggerActionResult DebuggerControlPlugin::setSourceAction() {
    auto path = std::get<std::string>(callbacks["get_focused_source"](0));
    return setSourceAction(path);
}

void signalCompilationResult(bool resp, const std::string& output, CallbackFunction callback) {
    callback(make_pair(resp ? 1 : -1, output));
    if (resp)
        ImGui::InsertNotification({ImGuiToastType_Success, 5000, "Compilation successful"});
    else
        ImGui::InsertNotification({ImGuiToastType_Error, 5000, "Compilation failed"});
}

DebuggerActionResult DebuggerControlPlugin::setSourceAction(const std::string& source) {
    bool changing_source = source != source_fn;
    if (setSource(source) != DebuggerActionResult::Success) {
        std::cerr << "Failed to set source" << std::endl;
        return DebuggerActionResult::FailedToSetSource;
    }
    callbacks["refresh_analyzer"](0);
    if (!debugger->compile(true)) {
        if (!changing_source)
            signalCompilationResult(false, debugger->getCompilationOutput(),
                                    callbacks["set_compilation_output"]);
        std::cerr << "Failed to compile source" << std::endl;
        return DebuggerActionResult::FailedToCompile;
    }
    if (!debugger->initialize(true)) {
        std::cerr << "Failed to initialize" << std::endl;
        return DebuggerActionResult::FailedToInitialize;
    }
    return DebuggerActionResult::Success;
}

DebuggerActionResult DebuggerControlPlugin::compileAction() {
    bool resp = debugger->compile();
    callbacks["refresh_analyzer"](0);
    signalCompilationResult(resp, debugger->getCompilationOutput(),
                            callbacks["set_compilation_output"]);
    if (resp)
        debugger->initialize();
    return resp ? DebuggerActionResult::Success : DebuggerActionResult::FailedToCompile;
}

int DebuggerControlPlugin::runAction() {
    callbacks["set_input"](0);
    if (!debugger->isCompiled())
        if (compileAction() != DebuggerActionResult::Success)
            return -2;
    return debugger->runExecution();
}