#include "debugger.h"

extern "C" {
void debugger_error_handler(WTStar::error_t *error, void *data) {
    static_cast<Debugger *>(data)->errorHandler(error);
}
}

Debugger::~Debugger() { destroy(); }

void Debugger::destroy() {
    destroyVm();
    binary.clear();
    if (ip)
        WTStar::driver_destroy(ip);
    ip = nullptr;
    if (ast)
        WTStar::ast_t_delete(ast);
    ast = nullptr;
    compiled = false;
    std::cerr << "destroy debugger" << std::endl;
}

void Debugger::destroyVm() {
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

bool Debugger::initialize(bool memory) {
    destroyVm();
    if (!memory) {
        binary.clear();
        if (!readBinary())
            return false;
    }
    env = WTStar::virtual_machine_t_new(binary.data(), static_cast<int>(binary.size()));
    if (!env) {
        std::cerr << "Failed to create virtual machine" << std::endl;
        return false;
    }
    for (auto &bp : breakpoints)
        addBreakpointToVm(bp);
    std::cerr << "created virtual machine" << std::endl;
    return true;
}

bool Debugger::setSource(const std::string &file) {
    if (file.empty()) {
        std::cerr << "empty file" << std::endl;
        destroy();
        source_fn.clear();
        return true;
    }
    if (!ends_with(file, ".wt"))
        return false;
    source_fn = file;
    binary_fn = source_fn.substr(0, source_fn.size() - 3) + ".wtb";
    std::cerr << "set source " << source_fn << " binary_fn " << binary_fn << std::endl;
    destroy();
    return true;
}

void Debugger::setInput(const std::string &input) { this->input = input; }

bool Debugger::readInput() {
    if (input.empty()) {
        std::cerr << "empty input" << std::endl;
        return false;
    }
    if (!canRun()) {
        std::cerr << "not compiled" << std::endl;
        return false;
    }
    WTStar::reader_t *in = reader_t_new(WTStar::READER_STRING, input.c_str());
    if (WTStar::read_input(in, env) != 0)
        destroyVm();
    WTStar::reader_t_delete(in);
    std::cerr << "read input success" << (env != nullptr) << std::endl;
    return env != nullptr;
}

std::string Debugger::getOutput() const {
    if (!env)
        return "";
    Writer wout;
    for (uint i = 0; i < env->n_out_vars; i++)
        WTStar::write_output(wout.w, env, i);
    WTStar::out_text(wout.w, "-----\nwork: %10d\ntime: %10d\n", env->W, env->T);
    return wout.read();
}

std::string Debugger::getCompilationOutput() const { return error_stream.str(); }

void Debugger::clearCompilationOutput() { error_stream.str(""); }

bool Debugger::compile(bool memory) {
    if (source_fn.empty())
        return false;

    ip = WTStar::include_project_t_new();
    WTStar::driver_init(ip);
    std::cerr << "compiling " << source_fn << std::endl;
    ast = WTStar::driver_parse(ip, source_fn.c_str(), debugger_error_handler, this);
    if (ast->error_occured) {
        std::cerr << "parse error" << std::endl;
        destroy();
        return false;
    }
    std::cerr << "parsed source into ast" << std::endl;

    // Writer eout;
    // WTStar::ast_debug_set_writer(eout.w);
    // WTStar::ast_debug_print(ast, 0);
    // std::cerr << "ast: " << eout.read() << std::endl;

    Writer out;
    if (!memory)
        out = Writer(binary_fn, "wb");
    int resp = WTStar::emit_code(ast, out.w, 0);
    std::cerr << "emited code into memory? " << memory << " or else " << binary_fn << " with resp "
              << resp << std::endl;
    if (resp) {
        std::cerr << "compilation UNsucessful" << std::endl;
        destroy();
        return false;
    }
    if (memory) {
        auto code_s = out.read();
        binary = code_t(code_s.begin(), code_s.end());
    } else
        compiled = true;
    std::cerr << "compilation sucessful" << std::endl;
    return true;
}

void Debugger::errorHandler(WTStar::error_t *error) {
    std::cerr << error->msg->str.base << std::endl;
    error_stream << error->msg->str.base;
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

bool Debugger::isCompiled() const { return compiled; }

bool Debugger::canRun() const { return compiled && env; }

std::pair<size_t, SourcePosition> Debugger::getSourcePosition() const {
    if (!canRun())
        return {-1, SourcePosition()};
    return {env->pc, findSourcePosition(env, env->pc)};
}

int Debugger::runExecution() {
    if (!compiled) {
        std::cerr << "not compiled" << std::endl;
        return -2;
    }
    if (!initialize() || !readInput())
        return -2;
    return continueExecution();
}

int Debugger::continueExecution() {
    if (!canRun())
        return -2;
    int resp = WTStar::execute(env, -1, 0, stop_on_bp);
    std::cerr << "continueExecution execute stopped with resp " << resp << std::endl;
    return resp;
}

void Debugger::pauseExecution() { std::cerr << "NOT IMPLEMENTED" << std::endl; }

void Debugger::stopExecution() { destroyVm(); }

int Debugger::stepOver() {
    if (!canRun())
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
    if (!canRun())
        return -2;
    int resp = WTStar::execute(env, -1, 0, 2 | stop_on_bp);
    uint32_t current_level = STACK_SIZE(env->frames, WTStar::frame_t *);
    std::cerr << "stepInto execute stopped with resp " << resp << " on line " << env->pc
              << " with level " << current_level << std::endl;
    return resp;
}

int Debugger::stepOut() {
    if (!canRun())
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