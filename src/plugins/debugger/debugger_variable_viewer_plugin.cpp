#include "debugger_variable_viewer_plugin.h"

DebuggerVariableViewerPlugin::DebuggerVariableViewerPlugin(Debugger *debugger)
    : debugger(debugger) {
    pluginType = PluginType::DebuggerVariableViewer;
    title = "Variable Viewer";
}

void DebuggerVariableViewerPlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin((title + "###" + std::to_string(getId())).c_str(),
                      immortal ? nullptr : &alive, ImGuiWindowFlags_None)) {
        ImGui::End();
        return;
    }

    WTStar::virtual_machine_t *env = debugger->env;
    if (!env) {
        ImGui::TextWrapped("No program running");
        ImGui::End();
        return;
    }

    Writer outw;
    ImGui::TextWrapped("yaay\n");

    ImGui::End();
}