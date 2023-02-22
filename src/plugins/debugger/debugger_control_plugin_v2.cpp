#include "debugger_control_plugin_v2.h"

void DebuggerControlPluginV2::show() {
    if (!shown)
        return;
    if (!imguiBegin(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        return;
    }

    {
        int resp = -1000;
        float spacing = 5.f, spacing2 = spacing * 2;

        if (ImGui::Button("")) {
            if (setSourceAction())
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
            auto tmp = debugger->stop_on_bp;
            debugger->stop_on_bp = false;
            resp = debugger->continueExecution();
            debugger->stop_on_bp = tmp;
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

        if (resp != -1000)
            callbacks["execution_progress"](resp);
        if (resp == -1)
            callbacks["set_output"](debugger->getOutput());
    }

    auto [pc, spos] = debugger->getSourcePosition();
    ImGui::TextWrapped("%04ld>%s:%ld", pc, spos.file.c_str(), spos.line);

    ImGui::End();
    seen = true;
}