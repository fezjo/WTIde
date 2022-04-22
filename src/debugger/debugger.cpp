#include "debugger.h"

Debugger::~Debugger()
{
    reset();
}

bool Debugger::setSource(const std::string &file)
{
    if (file.empty())
    {
        std::cerr << "empty file" << std::endl;
        reset();
        source_fn.clear();
        return true;
    }
    if (!ends_with(file, ".wt"))
        return false;
    source_fn = file;
    std::cerr << "set source " << source_fn << std::endl;
    reset();
    return true;
}

void Debugger::setInput(const std::string &input)
{
    this->input = input;
}

void Debugger::reset()
{
    binary_fn.clear();
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

bool Debugger::readBinary()
{
    if (binary_fn.empty())
        return false;
    std::ifstream ifs(binary_fn, std::ios::binary);
    if (!ifs.is_open())
    {
        std::cerr << "failed to open " << binary_fn << std::endl;
        return false;
    }
    ifs.seekg(0, std::ios::end);
    size_t size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    binary.resize(size);
    ifs.read(reinterpret_cast<char*>(binary.data()), size);
    ifs.close();
    std::cerr << "read " << binary_fn << " " << size << " bytes" << std::endl;
    return true;
}

bool Debugger::readInput()
{
    if (input.empty())
    {
        std::cerr << "empty input" << std::endl;
        return false;
    }
    if (!env)
    {
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

bool Debugger::compile()
{
    if (source_fn.empty())
        return false;

    binary_fn = source_fn.substr(0, source_fn.size() - 3) + ".wtb";
    std::cerr << "binary_fn " << binary_fn << std::endl;

    ip = WTStar::include_project_t_new();
    WTStar::driver_init(ip);
    ast = WTStar::driver_parse(ip, source_fn.c_str());
    std::cerr << "parsed source into ast" << std::endl;

    WTStar::writer_t *out = WTStar::writer_t_new(WTStar::WRITER_FILE);
    out->f = fopen(binary_fn.c_str(), "wb");
    int resp = WTStar::emit_code(ast, out, 0);
    std::cerr << "emited code into " << binary_fn << " with resp " << resp << std::endl;
    WTStar::writer_t_delete(out);
    
    if (resp)
    {
        std::cerr << "compilation UNsucessful" << std::endl;
        reset();
        return false;
    }
    std::cerr << "compilation sucessful" << std::endl;
    return true;
}

bool Debugger::initialize()
{
    reset();
    if (!compile() || !readBinary())
        return false;
    env = WTStar::virtual_machine_t_new(binary.data(), binary.size());
    std::cerr << "created virtual machine" << std::endl;
    return true;
}

// TODO
uint32_t Debugger::findInstructionNumber(const std::string &file, int line)
{
    std::cerr << "NOT IMPLEMENTED" << std::endl;
    return -1;
}

Breakpoint* Debugger::findBreakpoint(const std::string &file, int line)
{
    auto it = std::find_if(breakpoints.begin(), breakpoints.end(), [&](const Breakpoint &bp) {
        return bp.file == file && bp.line == line;
    });
    if (it == breakpoints.end())
        return nullptr;
    return &*it;
}

// TODO
std::vector<uint8_t> Debugger::compileCondition(const std::string &condition)
{
    if (condition.empty())
        return {};
    std::cerr << "NOT IMPLEMENTED" << std::endl;
    return {};
}

int Debugger::runExecution()
{
    if (!initialize() || !readInput())
        return -2;
    return continueExecution();
}

int Debugger::continueExecution()
{
    int resp = WTStar::execute(env, -1, 0, 1);
    std::cerr << "continueExecution execute stopped with resp " << resp << std::endl;
    return resp;
}

void Debugger::pauseExecution()
{
    std::cerr << "NOT IMPLEMENTED" << std::endl;
}

void Debugger::stopExecution()
{
    reset();
}

void Debugger::stepOver()
{
    std::cerr << "NOT IMPLEMENTED" << std::endl;
}

void Debugger::stepInto()
{
    std::cerr << "NOT IMPLEMENTED" << std::endl;
}

void Debugger::stepOut()
{
    std::cerr << "NOT IMPLEMENTED" << std::endl;
}

bool Debugger::setBreakpoint(const std::string &file, int line)
{
    return setBreakpointWithCondition(file, line, "");
}

bool Debugger::removeBreakpoint(const std::string &file, int line)
{
    auto bp = findBreakpoint(file, line);
    if (!bp)
        return false;
    WTStar::remove_breakpoint(env, bp->bp_pos);
    std::remove_if(breakpoints.begin(), breakpoints.end(), [&](const Breakpoint &bp) {
        return bp.file == file && bp.line == line;
    });
    return true;
}

void Debugger::removeAllBreakpoints()
{
    for (auto &bp : breakpoints)
        WTStar::remove_breakpoint(env, bp.bp_pos);
    breakpoints.clear();
}

bool Debugger::setBreakpointEnabled(const std::string &file, int line, bool enabled)
{
    auto bp = findBreakpoint(file, line);
    if (!bp)
        return false;
    if (!WTStar::enable_breakpoint(env, bp->bp_pos, enabled) == -1)
        return false;
    bp->enabled = enabled;
    return true;
}

bool Debugger::setBreakpointWithCondition(const std::string &file, int line, const std::string &condition)
{
    uint32_t bp_pos = findInstructionNumber(file, line);
    if (bp_pos == -1)
        return false;
    std::vector<uint8_t> code = compileCondition(condition);
    if (WTStar::add_breakpoint(env, bp_pos, code.empty() ? NULL : code.data(), code.size()) == -1)
        return false;
    breakpoints.push_back({file, line, condition, true, bp_pos});
    return true;
}