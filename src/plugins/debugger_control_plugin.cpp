#include "debugger_control_plugin.h"

DebuggerControlPlugin::DebuggerControlPlugin(Debugger *debugger) : debugger(debugger) {}

void DebuggerControlPlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin((title + "###" + std::to_string(getId())).c_str(),
                      immortal ? nullptr : &alive, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Set source"))
        setSource(source_fn);
    ImGui::SameLine();
    ImGui::InputText("##source_fn", &source_fn);

    if (ImGui::Button("Compile")) {
        debugger->clearCompilationOutput();
        debugger->compile();
        callbacks["refresh_analyzer"](0);
        callbacks["set_compilation_output"](debugger->getCompilationOutput());
    }
    ImGui::SameLine();
    if (ImGui::Button("Run")) {
        callbacks["set_input"](0);
        if (debugger->runExecution() == -1)
            callbacks["set_output"](debugger->getOutput());
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop")) {
        debugger->stopExecution();
    }

    if (ImGui::Button("Continue")) {
        if (debugger->continueExecution() == -1)
            callbacks["set_output"](debugger->getOutput());
    }
    ImGui::SameLine();
    if (ImGui::Button("Pause")) {
        debugger->pauseExecution();
    }

    ImGui::Text("Step");
    ImGui::SameLine();
    if (ImGui::Button("Over")) {
        debugger->stepOver();
    }
    ImGui::SameLine();
    if (ImGui::Button("Into")) {
        if (debugger->stepInto() == -1)
            callbacks["set_output"](debugger->getOutput());
    }
    ImGui::SameLine();
    if (ImGui::Button("Out")) {
        debugger->stepOut();
    }

    if (ImGui::Button("Breakpoints")) {
        ImGui::OpenPopup("Breakpoints");
    }
    ImGui::SameLine();
    ImGui::Checkbox("Stop on BP", &debugger->stop_on_bp);

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
                debugger->setBreakpoint(fileBuffer, lineBuffer);
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
                debugger->removeBreakpoint(fileBuffer, lineBuffer);
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove All")) {
            debugger->removeAllBreakpoints();
        }
        ImGui::EndPopup();
    }
    ImGui::End();
}

void DebuggerControlPlugin::setSource(const std::string &source) {
    source_fn = source;
    debugger->setSource(source);
}

void DebuggerControlPlugin::setInput(const std::string &input) { debugger->setInput(input); }