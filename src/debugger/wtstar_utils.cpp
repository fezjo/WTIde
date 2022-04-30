#include "wtstar_utils.h"

Writer::Writer() { w = WTStar::writer_t_new(WTStar::WRITER_STRING); }

Writer::Writer(const std::string &fn, const std::string &mode) {
    w = WTStar::writer_t_new(WTStar::WRITER_FILE);
    w->f = fopen(fn.c_str(), mode.c_str());
    if (ferror(w->f)) {
        WTStar::writer_t_delete(w);
        w = nullptr;
        throw std::runtime_error("failed to open file " + fn);
    }
}

Writer::Writer(Writer &&writer) {
    w = writer.w;
    writer.w = nullptr;
}

Writer &Writer::operator=(Writer &&writer) {
    if (w)
        WTStar::writer_t_delete(w);
    w = writer.w;
    writer.w = nullptr;
    return *this;
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

extern "C" {
void error_handler_callback(WTStar::error_t *error, void *data) {
    static_cast<ErrorHandler *>(data)->handle(error);
}
}

void ErrorHandler::clear() { ss.str(""); }

void ErrorHandler::write(const std::string &msg) { ss << msg; }

void ErrorHandler::handle(WTStar::error_t *error) { ss << error->msg->str.base << "\n"; }

std::string ErrorHandler::read() const { return ss.str(); }
