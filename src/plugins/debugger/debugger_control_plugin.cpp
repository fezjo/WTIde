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

bool DebuggerControlPlugin::setSourceAction(const std::string &source) {
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

void DebuggerControlPlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin((title + "###" + std::to_string(getId())).c_str(),
                      immortal ? nullptr : &alive, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return;
    }
    debugger->trace_on = true;
    static std::string fileBuffer = source_fn;

    if (ImGui::Button("Set source")) {
        setSourceAction(source_fn);
        fileBuffer = source_fn;
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, source_fn_color);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##source_fn", &source_fn,
                         ImGuiInputTextFlags_EnterReturnsTrue |
                             ImGuiInputTextFlags_AutoSelectAll)) {
        setSourceAction(source_fn);
        fileBuffer = source_fn;
    }
    ImGui::PopStyleColor();
    if (ImGui::IsItemEdited() || !seen)
        source_fn_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);

    {
        int resp = -1000;
        ImGui::BeginDisabled(debugger->getSource().empty());
        if (ImGui::Button("Compile")) {
            compileAction();
        }
        ImGui::SameLine();
        if (ImGui::Button("Run")) {
            resp = runAction();
        }
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!debugger->canRun());
        ImGui::SameLine();
        if (ImGui::Button("Destroy")) {
            debugger->stopExecution();
            setSourceAction(source_fn);
        }

        if (ImGui::Button("Continue")) {
            resp = debugger->continueExecution();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Trace", &debugger->trace_on);

        ImGui::Text("Step");
        ImGui::SameLine();
        if (ImGui::Button("Over")) {
            resp = debugger->stepOver();
        }
        ImGui::SameLine();
        if (ImGui::Button("Into")) {
            resp = debugger->stepInto();
        }
        ImGui::SameLine();
        if (ImGui::Button("Out")) {
            resp = debugger->stepOut();
        }
        ImGui::EndDisabled();
        if (resp == -1)
            callbacks["set_output"](debugger->getOutput());
    }

    {
        static int lineBuffer = 13;
        static std::string condition =
"int bn = A.size;\n\
sum = bn + 1;\n\
return bn;";

        ImGui::Text("Breakpoint:");
        ImGui::SameLine();
        ImGui::Checkbox("Stop on BP", &debugger->stop_on_bp);

        ImGui::BeginDisabled(!debugger->canAddBreakpoints());
        if (ImGui::Button("Add")) {
            ImGui::OpenPopup("Add Breakpoint");
        }
        if (ImGui::BeginPopup("Add Breakpoint")) {
            ImGui::InputText("File", &fileBuffer);
            ImGui::InputInt("Line", &lineBuffer);
            ImGui::InputTextMultiline("Condition", &condition, ImVec2(0, 0),
                                      ImGuiInputTextFlags_AllowTabInput);
            if (ImGui::Button("Add")) {
                auto res = debugger->setBreakpointWithCondition(fileBuffer, lineBuffer, condition);
                if (!res.first) {
                    ImGui::InsertNotification(
                        {ImGuiToastType_Error, 5000,
                         "Failed to set breakpoint\nSee console for details"});
                    callbacks["set_compilation_output"]("Failed to set breakpoint\n" + res.second);
                }
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove")) {
            ImGui::OpenPopup("Remove Breakpoint");
        }
        if (ImGui::BeginPopup("Remove Breakpoint")) {
            ImGui::InputText("File", &fileBuffer);
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
        ImGui::EndDisabled();
    }

    auto [pc, spos] = debugger->getSourcePosition();
    ImGui::TextWrapped("%04ld>%s:%ld(%d:%d:%d:%d)", pc, spos.file.c_str(), spos.line, spos.fl,
                       spos.fc, spos.ll, spos.lc);

    ImGui::End();
    seen = true;
}