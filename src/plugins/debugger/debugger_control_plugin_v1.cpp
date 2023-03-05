#include "debugger_control_plugin_v1.h"

bool DebuggerControlPluginV1::setSourceAction() {
    source_fn = std::get<std::string>(callbacks["get_focused_source"](0));
    return setSourceAction(source_fn);
}

bool DebuggerControlPluginV1::setSourceAction(const std::string& source) {
    if (!setSource(source)) {
        source_fn_color = ImVec4(1.0, 0.25, 0.25, 1.0);
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

void DebuggerControlPluginV1::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!imguiBegin(ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Set source")) {
        setSourceAction(source_fn);
        file_buffer = source_fn;
    }
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, source_fn_color);
    ImGui::SetNextItemWidth(-1);
    if (ImGui::InputText("##source_fn", &source_fn,
                         ImGuiInputTextFlags_EnterReturnsTrue |
                             ImGuiInputTextFlags_AutoSelectAll)) {
        setSourceAction(source_fn);
        file_buffer = source_fn;
    }
    ImGui::PopStyleColor();
    if (ImGui::IsItemEdited() || !seen)
        source_fn_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);

    {
        int resp = -1000;
        ImGui::BeginDisabled(debugger->getSource().empty());
        if (ImGui::Button("Compile"))
            compileAction();
        ImGui::SameLine();
        if (ImGui::Button("Run"))
            resp = runAction();
        ImGui::EndDisabled();

        ImGui::BeginDisabled(!debugger->canRun());
        ImGui::SameLine();
        if (ImGui::Button("Destroy")) {
            debugger->stopExecution();
            setSourceAction(source_fn);
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::Checkbox("Trace", &debugger->trace_on);

        ImGui::BeginDisabled(!debugger->canRun());
        if (ImGui::Button("Continue"))
            resp = debugger->continueExecution();
        ImGui::SameLine();
        ImGui::Text("Step");
        ImGui::SameLine();
        if (ImGui::Button("Over"))
            resp = debugger->stepOver();
        ImGui::SameLine();
        if (ImGui::Button("Into"))
            resp = debugger->stepInto();
        ImGui::SameLine();
        if (ImGui::Button("Out"))
            resp = debugger->stepOut();
        ImGui::EndDisabled();

        if (resp != -1000)
            callbacks["execution_progress"](resp);
        if (resp == -1)
            callbacks["set_output"](debugger->getOutput());
    }

    {
        static int line_buffer = 11;
        static std::string condition =
            "int bn = A.size;\n\
sum = bn + 1;\n\
return bn;";

        ImGui::Text("Breakpoint:");
        ImGui::SameLine();
        ImGui::Checkbox("Stop on BP", &debugger->stop_on_bp);

        ImGui::BeginDisabled(!debugger->canAddBreakpoints());
        if (ImGui::Button("Add"))
            ImGui::OpenPopup("Add Breakpoint");
        if (ImGui::BeginPopup("Add Breakpoint")) {
            ImGui::InputText("File", &file_buffer);
            ImGui::InputInt("Line", &line_buffer);
            ImGui::InputTextMultiline("Condition", &condition, ImVec2(0, 0),
                                      ImGuiInputTextFlags_AllowTabInput);
            if (ImGui::Button("Add")) {
                auto res = debugger->setBreakpoint(file_buffer, line_buffer, true, condition);
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
        if (ImGui::Button("Remove"))
            ImGui::OpenPopup("Remove Breakpoint");
        if (ImGui::BeginPopup("Remove Breakpoint")) {
            ImGui::InputText("File", &file_buffer);
            ImGui::InputInt("Line", &line_buffer);
            if (ImGui::Button("Remove"))
                debugger->removeBreakpoint(file_buffer, line_buffer);
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove All")) {
            // TODO
            // debugger->removeAllBreakpoints();
        }
        ImGui::EndDisabled();
    }

    auto [pc, spos] = debugger->getSourcePosition();
    ImGui::TextWrapped("%04ld>%s:%ld(%d:%d:%d:%d)", pc, spos.file.c_str(), spos.line, spos.fl,
                       spos.fc, spos.ll, spos.lc);

    ImGui::End();
    seen = true;
}