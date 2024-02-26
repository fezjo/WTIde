#include "debugger_variable_viewer_plugin.h"

Variable::Variable(WTStar::virtual_machine_t* env, WTStar::variable_info_t* info, bool global) {
    this->layout = WTStar::get_layout(info, env);
    this->global = global;
    this->addr = info->addr;
    this->n_dims = info->num_dim;
    this->from_code = info->from_code;
    this->stype = env->debug_info->types[info->type].name;
    this->sname = info->name;
}

Variable::~Variable() { free(layout.elems); }

Variable::Variable(Variable&& other) {
    this->layout.elems = nullptr;
    *this = std::move(other);
}

Variable& Variable::operator=(Variable&& other) {
    if (this == &other)
        return *this;
    free(layout.elems);

    auto l = std::exchange(other.layout, {});
    *this = other;
    this->layout = l;
    return *this;
}

Variable& Variable::fillThreadInfo(WTStar::virtual_machine_t* env, WTStar::thread_t* thr,
                                   uint base_addr) {
    if (!env)
        return *this;
    if (!thr)
        thr = STACK(STACK(env->threads, WTStar::stack_t*)[0], WTStar::thread_t*)[0];
    if (base_addr == -1u)
        base_addr = env->frame->base;

    this->raddr = !this->global * base_addr + this->addr;
    this->level = this->raddr < thr->mem_base;
    if (this->n_dims > 0) {
        this->dims.resize(this->n_dims);
        for (uint d = 0; d < this->n_dims; d++)
            this->dims[d] = static_cast<int>(WTStar::get_nth_dimension_size(thr, this->raddr, d));
    }

    Writer out;
    void* addr_data = WTStar::get_addr(thr, this->raddr, 4);
    if (this->n_dims == 0)
        WTStar::print_var(out.w, static_cast<uint8_t*>(addr_data), &this->layout);
    else {
        out.write("[");
        for (uint d = 0; d < this->n_dims; d++) {
            out.write(d ? ", " : "");
            out.write(std::to_string(WTStar::get_nth_dimension_size(thr, this->raddr, d)));
        }
        out.write("]");
        this->sdims = out.read();
        out.clear();
        WTStar::print_array(out.w, env, &this->layout, static_cast<int>(this->n_dims),
                            this->dims.data(), lval(addr_data, uint32_t), 0, 0);
    }
    this->svalue = out.read();
    return *this;
}

DebuggerVariableViewerPlugin::DebuggerVariableViewerPlugin(Debugger* debugger)
    : debugger(debugger) {
    pluginType = PluginType::DebuggerVariableViewer;
    title = "Variable Viewer";
}

int64_t DebuggerVariableViewerPlugin::getThreadParent(uint64_t tid) {
    auto* t = WTStar::get_thread(tid);
    if (!t)
        return -1;
    if (!t->parent)
        return 0;
    return static_cast<int64_t>(t->parent->tid);
}

// return active thread ids
std::vector<uint64_t> DebuggerVariableViewerPlugin::getThreadIds() {
    if (!debugger->isRunning())
        return {};
    auto* env = debugger->env;
    std::vector<uint64_t> tids;
    tids.reserve(static_cast<size_t>(env->a_thr));
    for (int t = 0; t < env->n_thr; t++)
        if (!env->thr[t]->returned)
            tids.push_back(env->thr[t]->tid);
    return tids;
}

// return name of pardo index variable
std::string DebuggerVariableViewerPlugin::getThreadIndexVariableName() {
    static const std::string not_found_name = "<index>";
    auto* env = debugger->env;
    auto debug_info = WTStar::getDebugInfo(env);
    if (!debugger->isRunning() || !env->n_thr || !env->thr[0]->parent)
        return "";
    if (!debug_info)
        return not_found_name;
    assert(env->stored_pc);
    int s = WTStar::code_map_find(debug_info->scope_map,
                                  static_cast<uint32_t>(env->stored_pc)); // current scope
    if (s < 0)
        return not_found_name;
    // go through all parent scopes
    for (uint sid = static_cast<uint>(debug_info->scope_map->val[s]); sid != MAP_SENTINEL;
         sid = debug_info->scopes[sid].parent) {
        WTStar::scope_info_t* scope = &debug_info->scopes[sid];
        for (uint i = 0; i < scope->n_vars; i++)
            // check if variable is in current frame and variable is pardo index
            if (env->thr[0]->mem_base >= env->frame->base && //
                scope->vars[i].addr == env->thr[0]->mem_base - env->frame->base) {
                return scope->vars[i].name;
            }
    }
    return not_found_name;
}

// return value of pardo index variable
int DebuggerVariableViewerPlugin::getThreadIndexVariableValue(uint64_t tid) {
    auto* t = WTStar::get_thread(tid);
    return t ? lval(t->mem->data, int32_t) : -1;
}

std::vector<Variable> DebuggerVariableViewerPlugin::getVariablesInScope(uint sid) {
    std::vector<Variable> variables;
    auto* env = debugger->env;
    auto debug_info = WTStar::getDebugInfo(env);
    if (!debug_info)
        return variables;

    auto* scope_info = &env->debug_info->scopes[sid];
    bool is_global = scope_info->parent == MAP_SENTINEL;
    for (uint v = 0; v < env->debug_info->scopes[sid].n_vars; v++) {
        auto* var_info = &scope_info->vars[v];
        variables.push_back(Variable(env, var_info, is_global));
    }

    return variables;
}

std::vector<std::vector<Variable>> DebuggerVariableViewerPlugin::getVisibleVariables(int pc) {
    std::vector<std::vector<Variable>> var_layers;
    auto* env = debugger->env;
    auto debug_info = WTStar::getDebugInfo(env);
    if (!debugger->isRunning() || !debug_info)
        return var_layers;

    pc = pc == -1 ? env->stored_pc : pc;
    assert(pc >= 0);
    int s = code_map_find(env->debug_info->scope_map, static_cast<uint32_t>(pc));
    if (s == -1)
        return var_layers;

    for (uint sid = static_cast<uint>(env->debug_info->scope_map->val[s]); sid != MAP_SENTINEL;
         sid = env->debug_info->scopes[sid].parent) {
        var_layers.push_back({});
        auto* scope_info = &env->debug_info->scopes[sid];
        for (auto& var : getVariablesInScope(sid)) {
            bool visible =
                static_cast<int>(var.from_code) <
                (scope_info->parent == MAP_SENTINEL ? env->last_global_pc : env->stored_pc);
            if (!visible)
                continue;
            // if (was_seen(var_info->name))
            //     continue;

            var.level = static_cast<int>(var_layers.size());
            var_layers.back().push_back(std::move(var));
        }
    }
    return var_layers;
}

static const ImGuiTreeNodeFlags default_treenode_flags =
    ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen;

bool showTableHeader(const std::string& title) {
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Hideable |
                            ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
                            ImGuiTableFlags_RowBg;
    if (!ImGui::BeginTable(title.c_str(), 5, flags))
        return false;
    // The first column will use the default _WidthStretch when ScrollX is Off and
    // _WidthFixed when ScrollX is On
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("Lvl",
                            ImGuiTableColumnFlags_DefaultHide | ImGuiTableColumnFlags_WidthFixed,
                            3 * TEXT_BASE_WIDTH);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 15 * TEXT_BASE_WIDTH);
    ImGui::TableSetupColumn("Addr[Abs/Rel]", ImGuiTableColumnFlags_DefaultHide);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
    return true;
}

void showTableVariableRow(const Variable& var) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", var.sname.c_str());
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%d", var.level);
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", var.stype.c_str());
    if (var.n_dims)
        ImGui::TextWrapped("[#%u:%s]", var.n_dims, var.sdims.c_str());
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%u/%u", var.raddr, var.addr);
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", var.svalue.c_str());
}

void showTableVariableLayer(std::vector<std::vector<Variable>>& var_layers, uint layer_i,
                            std::function<void(Variable& var)> actionRow) {
    if (layer_i >= var_layers.size())
        return;
    auto& var_layer = var_layers[layer_i];
    if (layer_i && var_layer.empty()) {
        showTableVariableLayer(var_layers, layer_i + 1, actionRow);
        return;
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    std::string title =
        std::string() + (layer_i ? "parent" : "locals") + "##var_layer" + std::to_string(layer_i);
    if (!ImGui::TreeNodeEx(title.c_str(), default_treenode_flags))
        return;
    for (auto& var : var_layer) {
        actionRow(var);
        // ImGui::TextWrapped("lvl=%d addr=%u type=%s ndim=%u%s name=%s val=%s",
        //                    var.level, var.addr, var.stype.c_str(),
        //                    var.n_dims, (var.sdims.empty() ? "" : " dims=" +
        //                    var.sdims).c_str(), var.sname.c_str(),
        //                    var.svalue.c_str());
    }
    showTableVariableLayer(var_layers, layer_i + 1, actionRow);
}

void DebuggerVariableViewerPlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!imguiBegin(ImGuiWindowFlags_None)) {
        ImGui::End();
        return;
    }

    WTStar::virtual_machine_t* env = debugger->env;
    if (!env) {
        ImGui::TextWrapped("No program running");
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("W=%9d T=%9d W/T=%5.2f", env->W, env->T, double(env->W) / env->T);

    ImGui::Separator();

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::CollapsingHeader("Globals")) {
        if (showTableHeader("globals")) {
            for (auto& var : getVariablesInScope(0)) {
                var.fillThreadInfo(env);
                showTableVariableRow(var);
            }
            ImGui::EndTable();
        }
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    {
        ImGui::Text("Thr:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("##thr_id", &watching_thr_id);
        watching_thr_id = std::max(0, std::min(env->n_thr - 1, watching_thr_id));

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Call stack")) {
            ulong call_stack_size = STACK_SIZE(env->frames, WTStar::frame_t*);
            ImGui::TextWrapped("Size: %lu", call_stack_size);
            int pc = env->stored_pc;
            for (int i = static_cast<int>(call_stack_size) - 1; i >= 0; --i) {
                auto frame = STACK(env->frames, WTStar::frame_t*)[i];
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (ImGui::TreeNodeEx(("var_layer" + std::to_string(i)).c_str(),
                                      default_treenode_flags, "%d: %s (base=%d ret_ins=%d)", i,
                                      frame->func_name, frame->base, pc)) {
                    auto var_layers = getVisibleVariables(pc);
                    if (showTableHeader("thread variables")) {
                        showTableVariableLayer(var_layers, 0, [&](Variable& var) {
                            var.fillThreadInfo(env, env->thr[watching_thr_id], frame->base);
                            showTableVariableRow(var);
                        });
                        ImGui::EndTable();
                    }
                }
                pc = static_cast<int>(frame->ret_addr);
            }
        }
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::NewLine();

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    auto var_layers = getVisibleVariables();
    if (ImGui::CollapsingHeader("Threads")) {
        auto index_name = getThreadIndexVariableName();
        for (auto tid : getThreadIds()) {
            auto* thr = WTStar::get_thread(tid);

            if (ImGui::Button(("Watch##" + std::to_string(tid)).c_str())) {
                for (int t = 0; t < env->n_thr; t++)
                    if (env->thr[t]->tid)
                        watching_thr_id = t;
            }
            ImGui::SameLine();
            bool open = ImGui::TreeNodeEx(("##thr_treenode" + std::to_string(tid)).c_str(),
                                          default_treenode_flags, "tid=%lu | par=%ld", tid,
                                          getThreadParent(tid));
            if (!index_name.empty()) {
                ImGui::SameLine();
                ImGui::Text(" | %s=%d", index_name.c_str(), getThreadIndexVariableValue(tid));
            }
            if (open) {
                if (showTableHeader("thread variables")) {
                    showTableVariableLayer(var_layers, 0, [&](Variable& var) {
                        var.fillThreadInfo(env, thr);
                        showTableVariableRow(var);
                    });
                    ImGui::EndTable();
                }
            }
        }
    }

    ImGui::End();
}

/*
    Work Time
    **local
    **parent -> parent -> parent
    **global
    parameters
    return value
*/