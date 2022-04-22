#include "debugger_control_plugin.h"

void DebuggerControlPlugin::update() {

}
void DebuggerControlPlugin::show() {
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(
        (title + "###" + std::to_string(getId())).c_str(),
        nullptr,
        ImGuiWindowFlags_HorizontalScrollbar
    ))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Set source"))
        debugger.setSource(source_fn);
    ImGui::SameLine();
    ImGui::InputText("##source_fn", &source_fn);

    if (ImGui::Button("Run")) {
        debugger.runExecution();
    }
    ImGui::SameLine();
    if (ImGui::Button("Continue")) {
        debugger.continueExecution();
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        debugger.pauseExecution();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        debugger.stopExecution();
    }

    ImGui::Text("Step");
    ImGui::SameLine();
    if (ImGui::Button("Over")) {
        debugger.stepOver();
    }
    ImGui::SameLine();
    if (ImGui::Button("Into")) {
        debugger.stepInto();
    }
    ImGui::SameLine();
    if (ImGui::Button("Out")) {
        debugger.stepOut();
    }

    if (ImGui::Button("Breakpoints")) {
        ImGui::OpenPopup("Breakpoints");
    }
    static char fileBuffer[256] = "";
    static int lineBuffer = 0;
    if (ImGui::BeginPopup("Breakpoints")) {
        if (ImGui::Button("Add")) {
            ImGui::OpenPopup("Add Breakpoint");
        }
        if (ImGui::BeginPopup("Add Breakpoint")) {
            ImGui::InputText("File", fileBuffer, sizeof(fileBuffer));
            ImGui::InputInt("Line", &lineBuffer);
            if (ImGui::Button("Add")) {
                debugger.setBreakpoint(fileBuffer, lineBuffer);
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            ImGui::OpenPopup("Remove Breakpoint");
        }
        if (ImGui::BeginPopup("Remove Breakpoint")) {
            ImGui::InputText("File", fileBuffer, sizeof(fileBuffer));
            ImGui::InputInt("Line", &lineBuffer);
            if (ImGui::Button("Remove")) {
                debugger.removeBreakpoint(fileBuffer, lineBuffer);
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove All")) {
            debugger.removeAllBreakpoints();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
void DebuggerControlPlugin::destroy() {

}