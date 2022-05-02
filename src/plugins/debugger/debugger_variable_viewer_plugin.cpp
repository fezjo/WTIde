#include "debugger_variable_viewer_plugin.h"

Variable::Variable(WTStar::variable_info_t *info, WTStar::virtual_machine_t *env,
                   WTStar::thread_t *thr) {
    this->layout = WTStar::get_layout(info, env);
    this->addr = info->addr;
    this->n_dims = info->num_dim;
    this->from_code = info->from_code;
    this->stype = env->debug_info->types[info->type].name;
    this->sname = info->name;
    fillThreadInfo(env, thr);
}

Variable::~Variable() {
    if (layout.elems)
        free(layout.elems);
}

Variable::Variable(Variable &&other) { *this = std::move(other); }

Variable &Variable::operator=(Variable &&other) {
    if (this == &other)
        return *this;
    this->layout = other.layout;
    this->addr = other.addr;
    this->n_dims = other.n_dims;
    this->from_code = other.from_code;
    this->stype = std::move(other.stype);
    this->sname = std::move(other.sname);
    this->dims = std::move(other.dims);
    this->sdims = std::move(other.sdims);
    this->svalue = std::move(other.svalue);
    this->level = other.level;
    other.layout = {};
    return *this;
}

void Variable::fillThreadInfo(WTStar::virtual_machine_t *env, WTStar::thread_t *thr) {
    if (!env)
        return;
    if (!thr)
        thr = STACK(STACK(env->threads, WTStar::stack_t *)[0], WTStar::thread_t *)[0];

    this->level = this->addr < thr->mem_base;
    if (this->n_dims > 0) {
        this->dims.resize(this->n_dims);
        for (uint d = 0; d < this->n_dims; d++)
            this->dims[d] = WTStar::get_nth_dimension_size(thr, this->addr, d);
    }

    Writer out;
    void *addr_data = WTStar::get_addr(thr, this->addr, 4);
    if (this->n_dims == 0)
        WTStar::print_var(out.w, static_cast<uint8_t *>(addr_data), &this->layout);
    else {
        out.write("[");
        for (uint d = 0; d < this->n_dims; d++) {
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
        WTStar::scope_info_t *scope = &debug_info->scopes[sid];
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
    auto *t = WTStar::get_thread(tid);
    return t ? lval(t->mem->data, int32_t) : -1;
}

std::vector<Variable> DebuggerVariableViewerPlugin::getVariablesInScope(uint sid) {
    std::vector<Variable> variables;
    auto *env = debugger->env;
    auto debug_info = WTStar::getDebugInfo(env);
    if (!debugger->isRunning() || !debug_info)
        return variables;

    auto *scope_info = &env->debug_info->scopes[sid];
    for (uint v = 0; v < env->debug_info->scopes[sid].n_vars; v++) {
        auto *var_info = &scope_info->vars[v];
        variables.push_back(Variable(var_info, env));
    }

    return variables;
}

std::vector<std::vector<Variable>> DebuggerVariableViewerPlugin::getVisibleVariables() {
    std::vector<std::vector<Variable>> var_layers;
    auto *env = debugger->env;
    auto debug_info = WTStar::getDebugInfo(env);
    if (!debugger->isRunning() || !debug_info)
        return var_layers;

    int s = code_map_find(env->debug_info->scope_map, env->stored_pc);
    if (s == -1)
        return var_layers;

    auto was_seen = [&](const std::string &var_name) {
        for (auto &layer : var_layers)
            for (auto &v : layer)
                if (v.sname == var_name)
                    return true;
        return false;
    };

    for (uint sid = env->debug_info->scope_map->val[s]; sid != MAP_SENTINEL;
         sid = env->debug_info->scopes[sid].parent) {
        var_layers.push_back({});
        auto *scope_info = &env->debug_info->scopes[sid];
        for (auto &var : getVariablesInScope(sid)) {
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

ImGuiTreeNodeFlags default_treenode_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen;

bool showTableHeader(const std::string &title) {
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
    ImGui::TableSetupColumn("Addr", ImGuiTableColumnFlags_DefaultHide);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();
    return true;
}

void showTableVariableRow(const Variable &var) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", var.sname.c_str());
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%d", var.level);
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", var.stype.c_str());
    if (var.n_dims) {
        ImGui::TextWrapped("[#%u:%s]", var.n_dims, var.sdims.c_str());
    }
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%u", var.addr);
    ImGui::TableNextColumn();
    ImGui::TextWrapped("%s", var.svalue.c_str());
}

void showTableVariableLayer(std::vector<std::vector<Variable>> &var_layers, WTStar::thread_t *thr,
                            uint layer_i, std::function<void(Variable &var)> actionRow) {
    if (layer_i >= var_layers.size())
        return;
    auto &var_layer = var_layers[layer_i];
    if (layer_i && var_layer.empty()) {
        showTableVariableLayer(var_layers, thr, layer_i + 1, actionRow);
        return;
    }

    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    std::string title =
        std::string() + (layer_i ? "parent" : "locals") + "##" + std::to_string(layer_i);
    if (!ImGui::TreeNodeEx(title.c_str(),
                           default_treenode_flags))
        return;
    for (auto &var : var_layer) {
        actionRow(var);
        // ImGui::TextWrapped("lvl=%d addr=%u type=%s ndim=%u%s name=%s val=%s",
        //                    var.level, var.addr, var.stype.c_str(),
        //                    var.n_dims, (var.sdims.empty() ? "" : " dims=" +
        //                    var.sdims).c_str(), var.sname.c_str(),
        //                    var.svalue.c_str());
    }
    showTableVariableLayer(var_layers, thr, layer_i + 1, actionRow);
};

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

    ImGui::TextWrapped("W=%9d T=%9d W/T=%5.2f", env->W, env->T, double(env->W) / env->T);

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNodeEx("Globals", default_treenode_flags)) {
        if (showTableHeader("globals")) {
            for (auto &var : getVariablesInScope(0)) {
                var.fillThreadInfo(env, nullptr);
                showTableVariableRow(var);
            }
            ImGui::EndTable();
        }
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    auto var_layers = getVisibleVariables();
    if (ImGui::TreeNodeEx("Threads", default_treenode_flags)) {
        auto index_name = getThreadIndexVariableName();
        for (auto tid : getThreadIds()) {
            auto *thr = WTStar::get_thread(tid);
            ImGui::Text("tid=%lu | par=%lu", tid, getThreadParent(tid));
            if (!index_name.empty()) {
                ImGui::SameLine();
                ImGui::TextWrapped(" | %s=%d", index_name.c_str(),
                                   getThreadIndexVariableValue(tid));
            }
            if (showTableHeader("thread variables")) {
                showTableVariableLayer(var_layers, thr, 0, [&](Variable &var) {
                    var.fillThreadInfo(env, thr);
                    showTableVariableRow(var);
                });
                ImGui::EndTable();
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