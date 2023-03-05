#include "debugger_control_plugin_v2.h"

void DebuggerControlPluginV2::show() {
    if (!shown)
        return;
    if (!imguiBegin(ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        ImGui::End();
        return;
    }

    auto tooltip = [](const std::string_view text) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("%s", text.data());
    };
    {
        int resp = -1000;
        float spacing = 5.f, spacing2 = spacing * 2;

        if (ImGui::Button("")) {
            if (setSourceAction())
                compileAction();
        }
        tooltip(
            "Set source file to currently focused editor tab and compile\nCurrent source file: " +
            source_fn);

        ImGui::BeginDisabled(debugger->getSource().empty());
        ImGui::SameLine(0, spacing);
        if (ImGui::Button(debugger->isRunning() ? "" : ""))
            resp = runAction();
        tooltip("Run/Rerun");
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!debugger->canRun());
        ImGui::SameLine(0, spacing2);
        if (ImGui::Button("")) {
            debugger->stopExecution();
            setSourceAction(source_fn);
        }
        tooltip("Stop");
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("")) {
            auto tmp = debugger->stop_on_bp;
            debugger->stop_on_bp = false;
            resp = debugger->continueExecution();
            debugger->stop_on_bp = tmp;
        }
        tooltip("Finish");
        ImGui::SameLine(0, spacing2);
        if (ImGui::Button(""))
            resp = debugger->continueExecution();
        tooltip("Continue");
        ImGui::SameLine(0, spacing);
        if (ImGui::Button(""))
            resp = debugger->stepOver();
        tooltip("Step Over");
        ImGui::SameLine(0, spacing);
        if (ImGui::Button(""))
            resp = debugger->stepInto();
        tooltip("Step Into");
        ImGui::SameLine(0, spacing);
        if (ImGui::Button(""))
            resp = debugger->stepOut();
        tooltip("Step Out");
        ImGui::EndDisabled();

        if (resp != -1000)
            callbacks["execution_progress"](resp);
        if (resp == -1)
            callbacks["set_output"](debugger->getOutput());
    }

    auto [pc, spos] = debugger->getSourcePosition();
    ImGui::TextWrapped("%04ld>%s:%ld", pc, spos.file.c_str(), spos.line);
    tooltip("Current source position (PC>file:line)");

    ImGui::End();
    seen = true;
}