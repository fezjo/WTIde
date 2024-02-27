#include "debugger.h"

SourcePosition::SourcePosition(WTStar::item_info_t* info, WTStar::virtual_machine_t* env)
    : fileid(info->fileid), fl(info->fl), fc(info->fc), ll(info->ll), lc(info->lc),
      file(env->debug_info->files[info->fileid]), line(info->fl), valid(true) {}

SourcePosition findSourcePosition(WTStar::virtual_machine_t* env, int instruction_number) {
    auto debug_info = WTStar::getDebugInfo(env);
    if (!debug_info)
        return SourcePosition();
    assert(env->stored_pc >= 0);
    int i =
        WTStar::code_map_find(debug_info->source_items_map, static_cast<uint32_t>(env->stored_pc));
    if (i < 0)
        return SourcePosition();
    int it = debug_info->source_items_map->val[i];
    if (it < 0)
        return SourcePosition();

    WTStar::item_info_t* item = &debug_info->items[it];
    SourcePosition res(item, env);
    res.file = normalize_path(res.file);
    return res;
}

uint Debugger::findInstructionNumber(const std::string& file, uint line) const {
    std::cerr << "findInstructionNumber " << file << " " << line << std::endl;
    auto debug_info = WTStar::getDebugInfo(env);
    if (!debug_info) // TODO add message
        return -1u;
    std::cerr << "debug_info?" << std::endl;
    if (!debug_info)
        return -1u;
    for (uint i = 0; i < debug_info->source_items_map->n; i++) {
        uint instr_i = debug_info->source_items_map->bp[i];
        int item_i = debug_info->source_items_map->val[i];
        if (item_i < 0)
            continue;
        WTStar::item_info_t* item = &debug_info->items[item_i];
        std::cerr << "item: " << item->fileid << " " << item->fl << " " << item->fc << " "
                  << item->ll << " " << item->lc << std::endl;
        std::string i_file(debug_info->files[item->fileid]);
        if (fs::equivalent(i_file, file) && item->fl == line)
            return instr_i;
    }
    return -1u;
}

using ast_scope_and_node = std::pair<WTStar::ast_node_t*, WTStar::ast_node_t*>;

ast_scope_and_node findAstScopeAndNode(WTStar::ast_t* ast,
                                       std::function<bool(WTStar::ast_node_t*)> predicate,
                                       WTStar::ast_node_t* node = nullptr,
                                       bool viable_scope = true) {
    std::cerr << "findAstScopeAndNode " << (void*)node << std::endl;
    if (!node)
        node = ast->root_node;
    std::cerr << "node " << node->node_type << " [" << node->loc.fl << ":" << node->loc.fc << ":"
              << node->loc.ll << ":" << node->loc.lc << "] instr" << node->code_from << "-"
              << node->code_to << std::endl;
    if (!predicate(node))
        return {};

    WTStar::scope_t* scope = nullptr;
    if (node->node_type == WTStar::AST_NODE_SCOPE) {
        scope = node->val.sc;
    } else if (node->node_type == WTStar::AST_NODE_FUNCTION) {
        scope = node->val.f->root_scope;
    } else if (node->node_type == WTStar::AST_NODE_STATEMENT) {
        for (int pi = 0; pi < 2; ++pi) {
            WTStar::ast_node_t* p = node->val.s->par[pi];
            if (!p)
                continue;
            ast_scope_and_node res =
                findAstScopeAndNode(ast, predicate, p, p->node_type != WTStar::AST_NODE_SCOPE);
            if (res.first)
                return res;
            if (res.second)             // if we found node but it does not have scope, then we have
                return {nullptr, node}; // also don't have scope so we return ourselves
        }
        // assert(!(bool)"unreachable");
        std::cerr << "could not find scope for statement" << std::endl;
        return {};
    } else
        return {nullptr, node};

    if (scope) {
        for (WTStar::ast_node_t* nex_node = scope->items; nex_node; nex_node = nex_node->next) {
            ast_scope_and_node res = findAstScopeAndNode(ast, predicate, nex_node);
            if (res.second) {
                if (viable_scope && !res.first)
                    res.first = node;
                return res;
            }
        }
    }
    return {};
}

std::pair<code_t, std::string> compileConditionInAst(WTStar::ast_t* ast,
                                                     const std::string& condition,
                                                     WTStar::ast_node_t* pos_scope_node) {
    if (condition.empty())
        return {};

    WTStar::include_project_t* ip = WTStar::include_project_t_new();
    WTStar::driver_init(ip);

    std::string cond_fn = "condition";
    WTStar::driver_set_file(ip, cond_fn.c_str(), condition.c_str());

    WTStar::ast_node_t* bp_scope_node =
        WTStar::ast_node_t_new(nullptr, WTStar::AST_NODE_SCOPE, pos_scope_node->val.sc);

    auto original_error_handler = ast->error_handler;
    auto original_error_handler_data = ast->error_handler_data;
    auto cleanup = [&]() {
        ast->error_handler = original_error_handler;
        ast->error_handler_data = original_error_handler_data;
        ast->current_scope = ast->root_node->val.sc;
        ast->error_occured = 0;
    };

    ErrorHandler error_handler;
    ast->error_handler = error_handler_callback;
    ast->error_handler_data = &error_handler;
    ast->current_scope = bp_scope_node->val.sc;
    ast = WTStar::driver_parse_from(ast, ip, cond_fn.c_str());

    Writer dout;
    WTStar::ast_debug_set_writer(dout.w);
    WTStar::ast_debug_print_node(0, bp_scope_node);
    std::cerr << dout.read() << std::endl;

    if (ast->error_occured) {
        cleanup();
        std::cerr << "Error occured while parsing condition" << std::endl;
        WTStar::driver_destroy(ip);
        WTStar::ast_node_t_delete(bp_scope_node);
        return {{}, error_handler.read()};
    }

    Writer out;
    WTStar::generating_breakpoint = true;
    int resp =
        WTStar::emit_code_scope_section(ast, pos_scope_node->base_addr_max, bp_scope_node, out.w);
    WTStar::generating_breakpoint = false;

    WTStar::driver_destroy(ip);
    WTStar::ast_node_t_delete(bp_scope_node);

    if (resp) {
        cleanup();
        error_handler.write("Error occured while compiling condition: emitting code failed\n");
        return {{}, error_handler.read()};
    }

    std::string code_s = out.read();
    code_t code(code_s.begin(), code_s.end());

    if (!( // this is only a heuristic
            code[code.size() - 1] == WTStar::ENDVM && code[code.size() - 2] == WTStar::MEM_FREE &&
            code[code.size() - 3] != WTStar::MEM_FREE)) {
        error_handler.write("Error occured while compiling condition: no final expression\n");
        return {code, error_handler.read()};
    }
    code.pop_back(); // remove last instruction (ENDVM)

    std::cerr << "breakpoint code_size " << code.size() << std::endl;

    return {code, error_handler.read()};
}

VM_Breakpoint* Debugger::findBreakpoint(const std::string& file, uint line) {
    auto it = std::find_if(breakpoints.begin(), breakpoints.end(), [&](const VM_Breakpoint& bp) {
        return bp.file == file && bp.line == line;
    });
    if (it == breakpoints.end())
        return nullptr;
    return &*it;
}

bool Debugger::addBreakpointToVm(VM_Breakpoint& bp) {
    if (WTStar::add_breakpoint(env, bp.bp_pos, bp.code.empty() ? nullptr : bp.code.data(),
                               static_cast<uint>(bp.code.size())) == -1) {
        std::cerr << "setBreakpointWithCondition failed" << std::endl;
        return false;
    }
    bp.vm_bp = WTStar::get_breakpoint(env, bp.bp_pos);
    return true;
}

std::pair<bool, std::string> Debugger::setBreakpoint(const std::string& file, uint line,
                                                     bool enabled, const std::string& condition) {
    auto bp = _setBreakpoint(file, line, enabled, condition);
    return {bp.active, bp.error};
}

VM_Breakpoint& Debugger::_setBreakpoint(const std::string file, uint line, bool enabled,
                                        const std::string condition) {
    auto it = breakpoints.erase(
        std::remove_if(breakpoints.begin(), breakpoints.end(),
                       [&](const VM_Breakpoint& bp) { return bp.file == file && bp.line == line; }),
        breakpoints.end());

    auto bp_pos = findInstructionNumber(file, line);
    if (it == breakpoints.end())
        it = std::upper_bound(breakpoints.begin(), breakpoints.end(), bp_pos,
                              [](uint pos, const VM_Breakpoint& bp) { return pos < bp.bp_pos; });
    VM_Breakpoint& bp = *breakpoints.insert(
        it, {{file, line, enabled, condition}, false, bp_pos, "not compiled", {}, nullptr});

    std::cerr << "setBreakpointWithCondition " << file << ":" << line << " " << bp.bp_pos
              << std::endl;
    if (bp.bp_pos == -1u) {
        bp.error = "Could not find corresponding instruction";
        bp.active = false;
        return bp;
    }

    if (!condition.empty()) {
        ast_scope_and_node scope_and_node = findAstScopeAndNode(ast, [&](WTStar::ast_node_t* node) {
            return node->code_from <= (int)bp.bp_pos && node->code_to >= (int)bp.bp_pos;
        });
        if (!scope_and_node.first) {
            std::cerr << "Could not find scope for breakpoint" << std::endl;
            bp.error = "Could not find corresponding scope";
            bp.active = false;
            return bp;
        } else
            std::cerr << "found scope and node id " << scope_and_node.first->val.sc->items->id
                      << " " << scope_and_node.second->id << std::endl;

        std::tie(bp.code, bp.error) = compileConditionInAst(ast, condition, scope_and_node.first);
        bp.active = bp.error.empty();
        std::cerr << "setBreakpointWithCondition added breakpoint at " << bp.bp_pos << std::endl;
        std::cerr << "bp.error " << bp.error << std::endl;
    } else {
        bp.error = "";
        bp.active = true;
    }

    bp.active &= enabled;
    if (env && bp.active)
        addBreakpointToVm(bp);
    return bp;
}

bool Debugger::setBreakpointEnabled(const std::string& file, uint line, bool enabled) {
    auto bp = findBreakpoint(file, line);
    if (!bp)
        return false;
    bp->enabled = enabled;
    if (env && WTStar::enable_breakpoint(env, bp->bp_pos, enabled) == -1)
        return false;
    bp->active = enabled;
    return true;
}

bool Debugger::removeBreakpoint(const std::string& file, uint line) {
    auto bp = findBreakpoint(file, line);
    if (!bp)
        return false;
    if (env && WTStar::remove_breakpoint(env, bp->bp_pos) == -1 && bp->active)
        return false;
    breakpoints.erase(
        std::remove_if(breakpoints.begin(), breakpoints.end(),
                       [&](const VM_Breakpoint& bp) { return bp.file == file && bp.line == line; }),
        breakpoints.end());
    return true;
}