#include "debugger_control_plugin_v2.h"

DebuggerControlPluginV2::DebuggerControlPluginV2(Debugger *debugger) : debugger(debugger) {
    pluginType = PluginType::PluginControl;
}

bool DebuggerControlPluginV2::setSource(const std::string &source) {
    if (!debugger->setSource(source))
        return false;
    source_fn = source;
    return true;
}

void DebuggerControlPluginV2::setInput(const std::string &input) { debugger->setInput(input); }

bool DebuggerControlPluginV2::setSourceAction(const std::string &source) {
    static auto set_source = [&]() {
        if (!setSource(source_fn)) {
            source_fn_color = ImVec4(1.0, 0.25, 0.25, 1.0);
            return false;
        }
        callbacks["refresh_analyzer"](0);
        return true;
    };
    if (!set_source()) {
        std::cerr << "Failed to set source" << std::endl;
        return false;
    }
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

bool DebuggerControlPluginV2::compileAction() {
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

int DebuggerControlPluginV2::runAction() {
    callbacks["set_input"](0);
    if (!debugger->isCompiled())
        if (!compileAction())
            return false;
    return debugger->runExecution();
}

void DebuggerControlPluginV2::show() {
    if (!shown)
        return;
    if (!imguiBegin(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        return;
    }

    {
        int resp = -1000;
        float spacing = 5.f;
        float spacing2 = spacing * 2;

        if (ImGui::Button("")) {
            setSourceAction(source_fn);
            compileAction();
        }

        ImGui::BeginDisabled(debugger->getSource().empty());
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("/")) {
            resp = runAction();
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!debugger->canRun());
        ImGui::SameLine(0, spacing2);
        if (ImGui::Button("")) {
            debugger->stopExecution();
            setSourceAction(source_fn);
        }
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("")) {
            debugger->stop_on_bp = false;
            resp = debugger->continueExecution();
        }
        ImGui::SameLine(0, spacing2);
        if (ImGui::Button("")) {
            resp = debugger->continueExecution();
        }
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("")) {
            resp = debugger->stepOver();
        }
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("")) {
            resp = debugger->stepInto();
        }
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("")) {
            resp = debugger->stepOut();
        }
        ImGui::EndDisabled();

        if (resp == -1)
            callbacks["set_output"](debugger->getOutput());
    }

    auto [pc, spos] = debugger->getSourcePosition();
    ImGui::TextWrapped("%04ld>%s:%ld", pc, spos.file.c_str(), spos.line);

    ImGui::End();
    seen = true;
}