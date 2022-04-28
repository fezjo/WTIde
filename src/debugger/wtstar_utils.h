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
    Writer(const std::string &fn, const std::string &mode);
    ~Writer();
    void clear();
    std::string read(size_t pos = 0, size_t len = -1u);

    WTStar::writer_t *w;
};

using code_t = std::vector<uint8_t>;

code_t readCode(const std::string &fn);