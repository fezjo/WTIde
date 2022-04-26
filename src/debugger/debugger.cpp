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
    binary.clear();
    if (env)
        WTStar::virtual_machine_t_delete(env);
    env = nullptr;
    if (ip)
        WTStar::driver_destroy(ip);
    ip = nullptr;
    if (ast)
        WTStar::ast_t_delete(ast);
    ast = nullptr;
    std::cerr << "reset" << std::endl;
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
        reset();
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
    reset();
    if (!readBinary())
        return false;
    env = WTStar::virtual_machine_t_new(binary.data(), static_cast<int>(binary.size()));
    std::cerr << "created virtual machine" << std::endl;
    return true;
}

void Debugger::errorHandler(WTStar::error_t *error) { error_stream << error->msg->str.base; }

// TODO
uint32_t Debugger::findInstructionNumber(const std::string &file, int line) {
    std::cerr << "NOT IMPLEMENTED" << std::endl;
    return -1;
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

Breakpoint *Debugger::findBreakpoint(const std::string &file, int line) {
    auto it = std::find_if(breakpoints.begin(), breakpoints.end(), [&](const Breakpoint &bp) {
        return bp.file == file && bp.line == line;
    });
    if (it == breakpoints.end())
        return nullptr;
    return &*it;
}

// TODO
std::vector<uint8_t> Debugger::compileCondition(const std::string &condition) {
    if (condition.empty())
        return {};
    std::cerr << "NOT IMPLEMENTED" << std::endl;
    return {};
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

void Debugger::stopExecution() { reset(); }

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

bool Debugger::setBreakpoint(const std::string &file, int line) {
    return setBreakpointWithCondition(file, line, "");
}

bool Debugger::setBreakpointEnabled(const std::string &file, int line, bool enabled) {
    auto bp = findBreakpoint(file, line);
    if (!bp)
        return false;
    if (WTStar::enable_breakpoint(env, bp->bp_pos, enabled) == -1)
        return false;
    bp->enabled = enabled;
    return true;
}

bool Debugger::setBreakpointWithCondition(const std::string &file, int line,
                                          const std::string &condition) {
    uint32_t bp_pos = findInstructionNumber(file, line);
    if (bp_pos == -1u)
        return false;
    std::vector<uint8_t> code = compileCondition(condition);
    if (WTStar::add_breakpoint(env, bp_pos, code.empty() ? NULL : code.data(),
                               static_cast<uint32_t>(code.size())) == -1)
        return false;
    breakpoints.push_back({file, line, condition, true, bp_pos});
    return true;
}

bool Debugger::removeBreakpoint(const std::string &file, int line) {
    auto bp = findBreakpoint(file, line);
    if (!bp)
        return false;
    WTStar::remove_breakpoint(env, bp->bp_pos);
    std::remove_if(breakpoints.begin(), breakpoints.end(),
                   [&](const Breakpoint &bp) { return bp.file == file && bp.line == line; });
    return true;
}

void Debugger::removeAllBreakpoints() {
    for (auto &bp : breakpoints)
        WTStar::remove_breakpoint(env, bp.bp_pos);
    breakpoints.clear();
}