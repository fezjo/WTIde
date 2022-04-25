#include "program_analyzer_plugin.h"

ProgramAnalyzerPlugin::ProgramAnalyzerPlugin(Debugger *debugger) : debugger(debugger) { refresh(); }

void ProgramAnalyzerPlugin::refresh() {}

void ProgramAnalyzerPlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin((title + "###" + std::to_string(getId())).c_str(),
                      immortal ? nullptr : &alive, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return;
    }

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

    WTStar::virtual_machine_t *env =
        WTStar::virtual_machine_t_new(code.data(), static_cast<int>(code.size()));
    if (!env) {
        ImGui::TextWrapped("Failed to create virtual machine, corrupted binary?");
        ImGui::End();
        return;
    }

    Writer outw;
    static auto writeTextWrappedAndClear = [](Writer &outw) {
        ImGui::TextWrapped("%s", outw.read().c_str());
        outw.clear();
    };

    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
        // if (ImGui::BeginTabItem("Current compilation output")) {
        //     ImGui::TextWrapped(debugger->getCompilationOutput().c_str());
        //     ImGui::EndTabItem();
        // }
        if (ImGui::BeginTabItem("Misc")) {
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
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Variables")) {
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
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Functions")) {
            ImGui::TextWrapped("TODO");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Code")) {
            ImGui::TextWrapped("Code size: %ld", code.size());
            print_code(outw.w, env->code, env->code_size);
            writeTextWrappedAndClear(outw);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}