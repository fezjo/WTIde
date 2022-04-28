#include "debugger.h"

extern "C" {
void debugger_error_handler(WTStar::error_t *error, void *data) {
    static_cast<Debugger *>(data)->errorHandler(error);
}
}

Writer::Writer() { w = WTStar::writer_t_new(WTStar::WRITER_STRING); }

Writer::Writer(const std::string &fn, const std::string &mode) {
    w = WTStar::writer_t_new(WTStar::WRITER_FILE);
    w->f = fopen(fn.c_str(), mode.c_str());
}

Writer::~Writer() { WTStar::writer_t_delete(w); }

void Writer::clear() {
    if (w->type == WTStar::WRITER_FILE)
        throw std::logic_error("Clearing file-based writer is not yet supported");
    w->str.base[0] = 0;
    w->str.ptr = 0;
}

std::string Writer::read(size_t pos, size_t len) {
    if (w->type == WTStar::WRITER_FILE)
        throw std::logic_error("Read from file-based writer is not yet supported");
    pos = std::min(pos, static_cast<size_t>(w->str.ptr));
    len = std::min(len, w->str.ptr - pos);
    return std::string(w->str.base + pos, len);
}

SourcePosition::SourcePosition(WTStar::item_info_t *info, WTStar::virtual_machine_t *env)
    : fileid(info->fileid), fl(info->fl), fc(info->fc), ll(info->ll), lc(info->lc),
      file(env->debug_info->files[info->fileid]), line(info->fl), valid(true) {}

SourcePosition findSourcePosition(WTStar::virtual_machine_t *env, int instruction_number) {
    if (!env->debug_info)
        return SourcePosition();
    int i = code_map_find(env->debug_info->source_items_map, env->pc);
    if (i < 0)
        return SourcePosition();
    int it = env->debug_info->source_items_map->val[i];
    if (it < 0)
        return SourcePosition();

    WTStar::item_info_t *item = &env->debug_info->items[it];
    SourcePosition res(item, env);
    return res;
}

code_t readCode(const std::string &fn) {
    std::ifstream ifs(fn, std::ios::binary);
    if (!ifs.is_open())
        throw std::runtime_error("Failed to open file " + fn);
    code_t code;
    ifs.seekg(0, std::ios::end);
    code.resize(ifs.tellg());
    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char *>(code.data()), code.size());
    return code;
}

Debugger::~Debugger() { reset(); }

bool Debugger::setSource(const std::string &file) {
    if (file.empty()) {
        std::cerr << "empty file" << std::endl;
        reset();
        source_fn.clear();
        return true;
    }
    if (!ends_with(file, ".wt"))
        return false;
    source_fn = file;
    std::cerr << "set source " << source_fn << std::endl;
    binary_fn = source_fn.substr(0, source_fn.size() - 3) + ".wtb";
    std::cerr << "binary_fn " << binary_fn << std::endl;
    reset();
    return true;
}

void Debugger::setInput(const std::string &input) { this->input = input; }

void Debugger::reset() {
    resetVm();
    if (ip)
        WTStar::driver_destroy(ip);
    ip = nullptr;
    if (ast)
        WTStar::ast_t_delete(ast);
    ast = nullptr;
    std::cerr << "reset" << std::endl;
}

void Debugger::resetVm() {
    binary.clear();
    if (env)
        WTStar::virtual_machine_t_delete(env);
    env = nullptr;
}

bool Debugger::readBinary() {
    if (binary_fn.empty())
        return false;
    try {
        binary = readCode(binary_fn);
    } catch (const std::exception &e) {
        std::cerr << "Failed to read binary file " << binary_fn << ": " << e.what() << std::endl;
        return false;
    }
    std::cerr << "read from " << binary_fn << " " << binary.size() << " bytes" << std::endl;
    return true;
}

bool Debugger::readInput() {
    if (input.empty()) {
        std::cerr << "empty input" << std::endl;
        return false;
    }
    if (!env) {
        std::cerr << "not compiled" << std::endl;
        return false;
    }
    WTStar::reader_t *in = reader_t_new(WTStar::READER_STRING, input.c_str());
    if (WTStar::read_input(in, env) != 0)
        resetVm();
    WTStar::reader_t_delete(in);
    std::cerr << "read input success" << (env != nullptr) << std::endl;
    return env != nullptr;
}

std::string Debugger::getOutput() {
    if (!env)
        return "";
    Writer wout;
    for (uint i = 0; i < env->n_out_vars; i++)
        WTStar::write_output(wout.w, env, i);
    WTStar::out_text(wout.w, "-----\nwork: %10d\ntime: %10d\n", env->W, env->T);
    return wout.read();
}

std::string Debugger::getCompilationOutput() { return error_stream.str(); }

void Debugger::clearCompilationOutput() { error_stream.str(""); }

bool Debugger::compile() {
    if (source_fn.empty())
        return false;

    ip = WTStar::include_project_t_new();
    WTStar::driver_init(ip);
    ast = WTStar::driver_parse(ip, source_fn.c_str(), debugger_error_handler, this);
    if (ast->error_occured) {
        std::cerr << "parse error" << std::endl;
        reset();
        return false;
    }
    std::cerr << "parsed source into ast" << std::endl;

    Writer out(binary_fn, "wb");
    int resp = WTStar::emit_code(ast, out.w, 0);
    std::cerr << "emited code into " << binary_fn << " with resp " << resp << std::endl;

    if (resp) {
        std::cerr << "compilation UNsucessful" << std::endl;
        reset();
        return false;
    }
    std::cerr << "compilation sucessful" << std::endl;
    return true;
}

bool Debugger::initialize() {
    resetVm();
    if (!readBinary())
        return false;
    env = WTStar::virtual_machine_t_new(binary.data(), static_cast<int>(binary.size()));
    std::cerr << "created virtual machine" << std::endl;
    return true;
}

void Debugger::errorHandler(WTStar::error_t *error) {
    std::cerr << error->msg->str.base << std::endl;
    error_stream << error->msg->str.base;
}

// TODO
uint Debugger::findInstructionNumber(const std::string &file, uint line) {
    if (!env)
        return -1u;
    if (!env->debug_info)
        return -1u;
    for (uint i = 0; i < env->debug_info->source_items_map->n; i++) {
        uint instr_i = env->debug_info->source_items_map->bp[i];
        int item_i = env->debug_info->source_items_map->val[i];
        if (item_i < 0)
            continue;
        WTStar::item_info_t *item = &env->debug_info->items[item_i];
        std::string i_file(env->debug_info->files[item->fileid]);
        std::cerr << "instruction to source " << i << " " << item_i << " " << instr_i << " "
                  << i_file << " " << item->fl << std::endl;
        if (i_file == file && item->fl == line) {
            std::cerr << "found item " << item_i << std::endl;
            return instr_i;
        }
    }
    return -1u;
}

// if (trace_on) {
//     printf("\nthread groups: ");
//     for (int i = 0; i < STACK_SIZE(env->threads, stack_t *); i++)
//     printf(" %lu ",
//             STACK_SIZE(STACK(env->threads, stack_t *)[i], thread_t *));
//     printf("\nenv->n_thr=%2d env->a_thr=%2d env->virtual_grps=%d\n",
//             env->n_thr, env->a_thr, env->virtual_grps);
//     if (env->a_thr > 0) {
//     printf("fbase=%d\n", env->frame->base);
//     for (int t = 0; t < env->n_thr; t++) {
//         printf("mem_base=%d size=%d parent=%lu", env->thr[t]->mem_base,
//                 env->thr[t]->mem->top, (unsigned long)env->thr[t]->parent);
//         printf("     [");
//         for (int i = 0; i < env->thr[t]->op_stack->top; i++)
//         printf("%d ", env->thr[t]->op_stack->data[i]);
//         printf("]\n");
//     }
//     printf("W=%d T=%d\n\n", env->W, env->T);
//     } else
//     printf("\n");
// }

std::pair<size_t, SourcePosition> Debugger::getSourcePosition() {
    if (!env)
        return {-1, SourcePosition()};
    return {env->pc, findSourcePosition(env, env->pc)};
}

Breakpoint *Debugger::findBreakpoint(const std::string &file, uint line) {
    auto it = std::find_if(breakpoints.begin(), breakpoints.end(), [&](const Breakpoint &bp) {
        return bp.file == file && bp.line == line;
    });
    if (it == breakpoints.end())
        return nullptr;
    return &*it;
}

using ast_scope_and_node = std::pair<WTStar::scope_t *, WTStar::ast_node_t *>;
ast_scope_and_node findAstScopeAndNode(WTStar::ast_t *ast,
                                       std::function<bool(WTStar::ast_node_t *)> predicate,
                                       WTStar::scope_t *scope = nullptr,
                                       WTStar::ast_node_t *node = nullptr,
                                       bool viable_scope = true) {
    static auto maybe_add_scope = [](WTStar::scope_t *scope, ast_scope_and_node &pair) {
        if (!pair.first)
            pair.first = scope;
        return pair;
    };
    std::cerr << "findAstScopeAndNode " << (void *)scope << " " << (void *)node << std::endl;
    if (!node && !scope)
        scope = ast->root_scope;
    if (scope) {
        std::cerr << "scope " << std::endl;
        for (WTStar::ast_node_t *node = scope->items; node; node = node->next) {
            ast_scope_and_node res = findAstScopeAndNode(ast, predicate, nullptr, node);
            if (res.second) {
                if (viable_scope)
                    return maybe_add_scope(scope, res);
                else
                    return res;
            }
        }
        return {};
    }
    std::cerr << "node " << node->node_type << " [" << node->loc.fl << ":" << node->loc.fc << ":"
              << node->loc.ll << ":" << node->loc.lc << "] instr" << node->code_from << "-"
              << node->code_to << std::endl;
    if (!predicate(node))
        return {};
    if (node->node_type == WTStar::AST_NODE_SCOPE) {
        return findAstScopeAndNode(ast, predicate, node->val.sc, nullptr);
    } else if (node->node_type == WTStar::AST_NODE_FUNCTION) {
        return findAstScopeAndNode(ast, predicate, node->val.f->root_scope, nullptr);
    } else if (node->node_type == WTStar::AST_NODE_STATEMENT) {
        for (int pi = 0; pi < 2; ++pi) {
            WTStar::ast_node_t *p = node->val.s->par[pi];
            ast_scope_and_node res;
            if (p->node_type == WTStar::AST_NODE_SCOPE)
                res = findAstScopeAndNode(ast, predicate, p->val.sc, nullptr, false);
            else
                res = findAstScopeAndNode(ast, predicate, nullptr, p);
            if (res.first)
                return res;
            if (res.second)             // if we found node but it does not have scope, then we have
                return {nullptr, node}; // also don't have scope so we return ourselves
        }
        assert(!(bool)"unreachable");
    } else {
        return {nullptr, node};
    }
}

code_t compileConditionInAst(WTStar::ast_t *ast, const std::string &condition,
                             WTStar::scope_t *scope_pos) {
    if (condition.empty())
        return {};

    WTStar::include_project_t *ip = WTStar::include_project_t_new();
    WTStar::driver_init(ip);

    std::string cond_fn = "condition";
    WTStar::driver_set_file(ip, cond_fn.c_str(), condition.c_str());

    WTStar::ast_node_t *bp_scope_node =
        WTStar::ast_node_t_new(NULL, WTStar::AST_NODE_SCOPE, scope_pos);

    ast->current_scope = bp_scope_node->val.sc;
    ast = WTStar::driver_parse_from(ast, ip, cond_fn.c_str());
    ast->current_scope = ast->root_scope;

    Writer dout;
    WTStar::ast_debug_set_writer(dout.w);
    WTStar::ast_debug_print_node(0, bp_scope_node);
    std::cerr << dout.read() << std::endl;

    if (ast->error_occured) {
        ast->error_occured = 0;
        std::cerr << "Error occured while parsing condition" << std::endl;
        WTStar::driver_destroy(ip);
        WTStar::ast_node_t_delete(bp_scope_node);
        return {};
    }

    Writer out;
    int resp = WTStar::emit_code_scope_section(ast, bp_scope_node->val.sc, out.w);

    WTStar::driver_destroy(ip);
    WTStar::ast_node_t_delete(bp_scope_node);

    if (resp)
        throw std::runtime_error("compilation failed");

    std::string code_s = out.read();
    code_t code(code_s.begin(), code_s.end());

    dout.clear();
    WTStar::print_code(dout.w, code.data(), (int)code.size());
    std::cerr << dout.read() << std::endl;

    if (!( // this is only a heuristic
            code[code.size() - 1] == WTStar::ENDVM && code[code.size() - 2] == WTStar::MEM_FREE &&
            code[code.size() - 3] != WTStar::MEM_FREE)) {
        throw std::runtime_error("no final expression");
    }
    code.pop_back(); // remove last instruction (ENDVM)

    std::cerr << "breakpoint code_size" << code.size() << std::endl;

    return code;
}

int Debugger::runExecution() {
    if (!initialize() || !readInput())
        return -2;
    return continueExecution();
}

int Debugger::continueExecution() {
    if (!env)
        return -2;
    int resp = WTStar::execute(env, -1, 1, stop_on_bp);
    std::cerr << "continueExecution execute stopped with resp " << resp << std::endl;
    return resp;
}

void Debugger::pauseExecution() { std::cerr << "NOT IMPLEMENTED" << std::endl; }

void Debugger::stopExecution() { resetVm(); }

int Debugger::stepOver() {
    if (!env)
        return -2;
    uint32_t initial_level = STACK_SIZE(env->frames, WTStar::frame_t *);
    int resp;
    while (true) {
        resp = WTStar::execute(env, -1, 0, 2 | stop_on_bp);
        if (resp != -11)
            break;
        uint32_t current_level = STACK_SIZE(env->frames, WTStar::frame_t *);
        std::cerr << "stepOver resp " << resp << " level " << current_level << "[" << initial_level
                  << "]" << std::endl;
        if (current_level <= initial_level)
            break;
    }
    std::cerr << "stepOver execute stopped with resp " << resp << " on line " << env->pc
              << std::endl;
    return resp;
}

int Debugger::stepInto() {
    if (!env)
        return -2;
    int resp = WTStar::execute(env, -1, 0, 2 | stop_on_bp);
    uint32_t current_level = STACK_SIZE(env->frames, WTStar::frame_t *);
    std::cerr << "stepInto execute stopped with resp " << resp << " on line " << env->pc
              << " with level " << current_level << std::endl;
    return resp;
}

int Debugger::stepOut() {
    if (!env)
        return -2;
    uint32_t initial_level = STACK_SIZE(env->frames, WTStar::frame_t *);
    int resp;
    while (true) {
        resp = WTStar::execute(env, -1, 0, 4 | stop_on_bp);
        if (resp != -11)
            break;
        uint32_t current_level = STACK_SIZE(env->frames, WTStar::frame_t *);
        std::cerr << "stepOut resp " << resp << " level " << current_level << "[" << initial_level
                  << "]" << std::endl;
        if (current_level < initial_level)
            break;
    }
    std::cerr << "stepOut execute stopped with resp " << resp << " on line " << env->pc
              << std::endl;
    return resp;
}

bool Debugger::setBreakpoint(const std::string &file, uint line) {
    return setBreakpointWithCondition(file, line, "");
}

bool Debugger::setBreakpointEnabled(const std::string &file, uint line, bool enabled) {
    auto bp = findBreakpoint(file, line);
    if (!bp)
        return false;
    if (WTStar::enable_breakpoint(env, bp->bp_pos, enabled) == -1)
        return false;
    bp->enabled = enabled;
    return true;
}

bool Debugger::setBreakpointWithCondition(const std::string &file, uint line,
                                          const std::string &condition) {
    uint bp_pos = findInstructionNumber(file, line);
    if (bp_pos == -1u)
        return false;
    ast_scope_and_node scope_and_node = findAstScopeAndNode(ast, [&](WTStar::ast_node_t *node) {
        return node->code_from <= (int)bp_pos && node->code_to >= (int)bp_pos;
    });
    std::cerr << "found scope and node id " << scope_and_node.first->items->id << " "
              << scope_and_node.second->id << std::endl;
    std::vector<uint8_t> code = compileConditionInAst(ast, condition, scope_and_node.first);
    if (WTStar::add_breakpoint(env, bp_pos, code.empty() ? NULL : code.data(),
                               static_cast<uint>(code.size())) == -1) {
        std::cerr << "setBreakpointWithCondition failed" << std::endl;
        return false;
    }
    std::cerr << "setBreakpointWithCondition added breakpoint at " << bp_pos << std::endl;
    breakpoints.push_back(
        {file, line, condition, bp_pos, "", code, WTStar::get_breakpoint(env, bp_pos), true}); // TODO compilation error
    return true;
}

bool Debugger::removeBreakpoint(const std::string &file, uint line) {
    auto bp = findBreakpoint(file, line);
    if (!bp)
        return false;
    if (WTStar::remove_breakpoint(env, bp->bp_pos) == -1)
        return false;
    breakpoints.erase(
        std::remove_if(breakpoints.begin(), breakpoints.end(),
                       [&](const Breakpoint &bp) { return bp.file == file && bp.line == line; }),
        breakpoints.end());
    return true;
}

void Debugger::removeAllBreakpoints() {
    for (auto &bp : breakpoints)
        WTStar::remove_breakpoint(env, bp.bp_pos);
    breakpoints.clear();
}