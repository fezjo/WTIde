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

    if (ImGui::Button("Run")) {
        runExecution();
    }
    ImGui::SameLine();
    if (ImGui::Button("Continue")) {
        continueExecution();
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        stopExecution();
    }

    ImGui::Text("Step");
    ImGui::SameLine();
    if (ImGui::Button("Over")) {
        stepOver();
    }
    ImGui::SameLine();
    if (ImGui::Button("Into")) {
        stepInto();
    }
    ImGui::SameLine();
    if (ImGui::Button("Out")) {
        stepOut();
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
                setBreakpoint(fileBuffer, lineBuffer);
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
                removeBreakpoint(fileBuffer, lineBuffer);
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove All")) {
            removeAllBreakpoints();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}
void DebuggerControlPlugin::destroy() {

}

void DebuggerControlPlugin::runExecution() {

}
void DebuggerControlPlugin::continueExecution() {

}
void DebuggerControlPlugin::stopExecution() {

}

void DebuggerControlPlugin::stepOver() {

}
void DebuggerControlPlugin::stepInto() {

}
void DebuggerControlPlugin::stepOut() {

}

void DebuggerControlPlugin::setBreakpoint(const std::string &file, int line) {

}
void DebuggerControlPlugin::removeBreakpoint(const std::string &file, int line) {

}
void DebuggerControlPlugin::removeAllBreakpoints() {

}
void DebuggerControlPlugin::setBreakpointEnabled(const std::string &file, int line, bool enabled) {

}
void DebuggerControlPlugin::setBreakpointCondition(const std::string &file, int line, const std::string &condition) {

}
