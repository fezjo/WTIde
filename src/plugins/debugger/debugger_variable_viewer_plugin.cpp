#include "debugger_variable_viewer_plugin.h"

Variable::Variable(WTStar::variable_info_t *info, WTStar::virtual_machine_t *env,
                   WTStar::thread_t *thr) {
    this->layout = WTStar::get_layout(info, env);
    this->addr = info->addr;
    this->n_dims = info->num_dim;
    this->stype = env->debug_info->types[info->type].name;
    this->sname = info->name;
    fillThreadInfo(env, thr);
}

void Variable::fillThreadInfo(WTStar::virtual_machine_t *env, WTStar::thread_t *thr) {
    this->level = this->addr < thr->mem_base;
    if (this->n_dims > 0) {
        this->dims.resize(this->n_dims);
        for (int d = 0; d < this->n_dims; d++)
            this->dims[d] = WTStar::get_nth_dimension_size(thr, this->addr, d);
    }

    Writer out;
    void *addr_data = get_addr(thr, this->addr, 4);
    if (this->n_dims == 0)
        WTStar::print_var(out.w, reinterpret_cast<uint8_t *>(addr_data), &this->layout);
    else {
        out.write("[");
        for (int d = 0; d < this->n_dims; d++) {
            out.write(d ? ", " : "");
            out.write(std::to_string(WTStar::get_nth_dimension_size(thr, this->addr, d)));
        }
        out.write("]");
        this->sdims = out.read();
        out.clear();
        WTStar::print_array(out.w, env, &this->layout, this->n_dims, this->dims.data(),
                            lval(addr_data, uint32_t), 0, 0);
    }
    this->svalue = out.read();
    if (this->layout.elems)
        free(this->layout.elems);
}

DebuggerVariableViewerPlugin::DebuggerVariableViewerPlugin(Debugger *debugger)
    : debugger(debugger) {
    pluginType = PluginType::DebuggerVariableViewer;
    title = "Variable Viewer";
}

int64_t DebuggerVariableViewerPlugin::getThreadParent(uint64_t tid) {
    auto *t = WTStar::get_thread(tid);
    if (!t)
        return -1;
    if (!t->parent)
        return 0;
    return t->parent->tid;
}

// return active thread ids
std::vector<uint64_t> DebuggerVariableViewerPlugin::getThreadIds() {
    if (!debugger->isRunning())
        return {};
    auto *env = debugger->env;
    std::vector<uint64_t> tids;
    tids.reserve(env->a_thr);
    for (int t = 0; t < env->n_thr; t++)
        if (!env->thr[t]->returned)
            tids.push_back(env->thr[t]->tid);
    return tids;
}

// return name of pardo index variable
std::string DebuggerVariableViewerPlugin::getThreadIndexVariableName() {
    static std::string not_found_name = "nonlocal_index";
    auto *env = debugger->env;
    auto debug_info = WTStar::getDebugInfo(env);
    if (!debugger->isRunning() || !env->n_thr || !env->thr[0]->parent)
        return "";
    if (!debug_info)
        return not_found_name;
    int s = WTStar::code_map_find(debug_info->scope_map, env->stored_pc); // current scope
    if (s < 0)
        return not_found_name;
    // go through all parent scopes
    for (uint sid = debug_info->scope_map->val[s]; sid != MAP_SENTINEL;
         sid = debug_info->scopes[sid].parent) {
        WTStar::scope_info_t *curren_scope = &debug_info->scopes[sid];
        for (uint i = 0; i < curren_scope->n_vars; i++)
            // check if variable is in current frame and variable is pardo index
            if (env->thr[0]->mem_base >= env->frame->base && //
                curren_scope->vars[i].addr == env->thr[0]->mem_base - env->frame->base) {
                return curren_scope->vars[i].name;
            }
    }
    return not_found_name;
}

// return value of pardo index variable
int DebuggerVariableViewerPlugin::getThreadIndexVariableValue(uint64_t tid) {
    auto *t = WTStar::get_thread(tid);
    return t ? lval(t->mem->data, int32_t) : -1;
}

void DebuggerVariableViewerPlugin::show() {
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
        ImGui::TextWrapped("No program running");
        ImGui::End();
        return;
    }

    Writer outw;
    ImGui::TextWrapped("yaay\n");

    if (ImGui::TreeNode("Threads")) {
        auto index_name = getThreadIndexVariableName();
        for (auto tid : getThreadIds()) {
            ImGui::Text("tid=%lu | par=%lu", tid, getThreadParent(tid));
            if (!index_name.empty()) {
                ImGui::SameLine();
                ImGui::TextWrapped(" | %s=%d", index_name.c_str(),
                                   getThreadIndexVariableValue(tid));
            }
        }
        ImGui::TreePop();
    }

    ImGui::End();
}

/*
    Work Time
    local
    parent -> parent -> parent
    global
    parameters
    return value
*/