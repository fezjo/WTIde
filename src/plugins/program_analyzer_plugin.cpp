#include "program_analyzer_plugin.h"

ProgramAnalyzerPlugin::ProgramAnalyzerPlugin(Debugger *debugger) : debugger(debugger) {
    pluginType = PluginType::ProgramAnalyzer;
    refresh();
}

void ProgramAnalyzerPlugin::refresh() {}

void ProgramAnalyzerPlugin::show() {
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
        std::string fn = debugger->binary_fn;
        if (fn.empty()) {
            ImGui::TextWrapped("No binary file");
            ImGui::End();
            return;
        }

        code_t code;
        try {
            code = readCode(fn);
        } catch (const std::exception &e) {
            ImGui::TextWrapped("Failed to read binary file %s: %s", fn.c_str(), e.what());
            ImGui::End();
            return;
        }

        env = WTStar::virtual_machine_t_new(code.data(), static_cast<int>(code.size()));
        if (!env) {
            ImGui::TextWrapped("Failed to create virtual machine, corrupted binary?");
            ImGui::End();
            return;
        }
    }

    Writer outw;
    static auto writeTextWrappedAndClear = [](Writer &outw) {
        ImGui::TextWrapped("%s", outw.read().c_str());
        outw.clear();
    };

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_Reorderable)) {
        // if (ImGui::BeginTabItem("Current compilation output")) {
        //     ImGui::TextWrapped(debugger->getCompilationOutput().c_str());
        //     ImGui::EndTabItem();
        // }
        if (ImGui::BeginTabItem("Misc")) {
            ImGui::BeginChild("##child");
            WTStar::dump_header(outw.w, env);
            writeTextWrappedAndClear(outw);

            bool has_debug = env->debug_info;
            if (has_debug) {
                ImGui::SetNextItemOpen(has_debug, ImGuiCond_Once);
                if (ImGui::TreeNode("Debug info")) {
                    WTStar::out_text(outw.w, "source files:\n\t");
                    for (uint32_t i = 0; i < env->debug_info->n_files; ++i)
                        WTStar::out_text(outw.w, "%s ", env->debug_info->files[i]);
                    WTStar::out_text(outw.w, "\n");
                    WTStar::print_types(outw.w, env);
                    writeTextWrappedAndClear(outw);
                    ImGui::TreePop();
                }
            } else
                ImGui::TextDisabled("Debug info");
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Variables")) {
            ImGui::BeginChild("##child");
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Input")) {
                WTStar::print_io_vars(outw.w, env, env->n_in_vars, env->in_vars);
                writeTextWrappedAndClear(outw);
                ImGui::TreePop();
            }
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Output")) {
                WTStar::print_io_vars(outw.w, env, env->n_out_vars, env->out_vars);
                writeTextWrappedAndClear(outw);
                ImGui::TreePop();
            }
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Root")) {
                WTStar::print_root_vars(outw.w, env);
                writeTextWrappedAndClear(outw);
                ImGui::TreePop();
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Functions")) {
            ImGui::BeginChild("##child");
            ImGui::TextWrapped("TODO");
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Code")) {
            ImGui::BeginChild("##child");
            ImGui::TextWrapped("Code size: %d", env->code_size);
            print_code(outw.w, env->code, env->code_size);
            writeTextWrappedAndClear(outw);
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Breakpoints")) {
            ImGui::BeginChild("##child");
            ImGui::TextWrapped("Breakpoints: %ld", debugger->breakpoints.size());
            for (auto &bp : debugger->breakpoints) {
                bool is_editing = edit_bp.bp_pos == bp.bp_pos;
                bool action_update = false, action_remove = false;
                bool enabled = bp.enabled;

                std::string &file = is_editing ? edit_bp.file : bp.file;
                int line = static_cast<int>(is_editing ? edit_bp.line : bp.line);
                std::string &condition = is_editing ? edit_bp.condition : bp.condition;
                ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_ReadOnly * !is_editing;

                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNode(std::to_string(bp.bp_pos).c_str(), "%s:%d", bp.file.c_str(),
                                    bp.line)) {
                    // TODO compilation error
                    ImGui::Checkbox("Enabled", &enabled);
                    ImGui::SameLine();
                    if (ImGui::Button("Edit")) {
                        if (!is_editing)
                            edit_bp = bp;
                        else
                            edit_bp = {};
                    }
                    ImGui::SameLine();
                    ImGui::BeginDisabled(!is_editing);
                    if (ImGui::Button("Update"))
                        action_update = is_editing;
                    ImGui::EndDisabled();
                    ImGui::SameLine();
                    if (ImGui::Button("Remove"))
                        action_remove = true;

                    ImGui::Text("File:");
                    ImGui::SameLine();
                    ImGui::InputText("##file", &file, input_flags);

                    ImGui::Text("Line:");
                    ImGui::SameLine();
                    ImGui::InputInt("##line", &line, 1, 10,
                                    input_flags | ImGuiInputTextFlags_CharsDecimal);

                    if (ImGui::TreeNode("Condition:")) {
                        ImGui::InputTextMultiline("##condition", &condition, ImVec2(-1, 0),
                                                  input_flags);
                        ImGui::TreePop();
                    }

                    ImGui::SetNextItemOpen(true, ImGuiCond_Once); // TODO testing hide
                    if (ImGui::TreeNode("VM")) {
                        if (!bp.vm_bp)
                            ImGui::Text("No VM breakpoint");
                        else {
                            ImGui::TextWrapped("ID: %d", bp.vm_bp->id);
                            ImGui::TextWrapped("BREAK position: %5d", bp.vm_bp->bp_pos);
                            ImGui::TextWrapped("Code  position: %5d", bp.vm_bp->code_pos);
                            ImGui::TextWrapped("Code  size: %5d", bp.vm_bp->code_size);
                            if (ImGui::TreeNode("Instructions")) {
                                outw.clear();
                                WTStar::print_code(outw.w, env->code + bp.vm_bp->code_pos,
                                                static_cast<int>(bp.vm_bp->code_size));
                                ImGui::TextWrapped("%s", outw.read().c_str());
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }

                    if (!bp.error.empty()) {
                        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                        if (ImGui::TreeNode("Compilation error")) {
                            ImGui::TextWrapped("%s", bp.error.c_str());
                            ImGui::TreePop();
                        }
                    }

                    ImGui::TreePop();
                }

                if (is_editing)
                    bp.line = static_cast<uint>(line);

                if (enabled != bp.enabled)
                    debugger->setBreakpointEnabled(bp.file, bp.line, enabled);
                if (action_update || action_remove)
                    debugger->removeBreakpoint(bp.file, bp.line);
                if (action_update) {
                    debugger->setBreakpointWithCondition(edit_bp.file, edit_bp.line,
                                                         edit_bp.condition);
                    edit_bp = {};
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}