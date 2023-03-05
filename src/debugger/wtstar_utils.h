#include "../utils.h"

namespace WTStar {
extern "C" {
#include "ast.h"
#include "ast_debug_print.h"
#include "code_generation.h"
#include "debug.h"
#include "driver.h"
#include "errors.h"
#include "vm.h"
}
} // namespace WTStar

class Writer {
public:
    Writer();
    Writer(const std::string& fn, const std::string& mode);
    Writer(Writer&& writer);
    Writer& operator=(Writer&& writer);
    ~Writer();
    void clear();
    void write(const std::string& s);
    std::string read(size_t pos = 0, size_t len = -1ul);

    WTStar::writer_t* w;
};

using code_t = std::vector<uint8_t>;

code_t readCode(const std::string& fn);

extern "C" {
void error_handler_callback(WTStar::error_t* error, void* data);
}

class ErrorHandler {
public:
    ErrorHandler() = default;
    void handle(WTStar::error_t* error);
    void clear();
    void write(const std::string& msg);
    std::string read() const;

    bool to_cerr = false;
    std::stringstream ss;
};

namespace WTStar {
debug_info_t* getDebugInfo(virtual_machine_t* env);
} // namespace WTStar