#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "../../debugger/debugger.h" // TODO why does it need to be at the end

struct Variable {
    WTStar::input_layout_item_t layout;
    uint addr;
    uint n_dims;
    std::string stype;
    std::string sname;

    std::vector<int> dims;
    std::string sdims;
    std::string svalue;

    int level;

    ~Variable() {
        if (layout.elems)
            free(layout.elems);
    }
    Variable(WTStar::variable_info_t *info, WTStar::virtual_machine_t *env, WTStar::thread_t *thr);
    void fillThreadInfo(WTStar::virtual_machine_t *env, WTStar::thread_t *thr);
};

class DebuggerVariableViewerPlugin : public IPlugin {
public:
    DebuggerVariableViewerPlugin(Debugger *debugger);
    void show() override;

    void refresh();

    int64_t getThreadParent(uint64_t tid);
    std::vector<uint64_t> getThreadIds();
    std::string getThreadIndexVariableName();
    int getThreadIndexVariableValue(uint64_t tid);

protected:
    Debugger *debugger;
};