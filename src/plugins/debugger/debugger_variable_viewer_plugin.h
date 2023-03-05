#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "../../debugger/debugger.h" // TODO why does it need to be at the end

struct Variable {
    WTStar::input_layout_item_t layout;
    bool global;
    uint addr;
    uint n_dims;
    uint from_code;
    std::string stype;
    std::string sname;

    uint raddr;
    std::vector<int> dims;
    std::string sdims;
    std::string svalue;

    int level;

    ~Variable();
    Variable(Variable&& other);
    Variable& operator=(Variable&& other);
    Variable(WTStar::virtual_machine_t* env, WTStar::variable_info_t* info, bool global);
    Variable& fillThreadInfo(WTStar::virtual_machine_t* env, WTStar::thread_t* thr = nullptr,
                             uint base_addr = -1u);

protected:
    Variable& operator=(Variable& other) = default;
};

class DebuggerVariableViewerPlugin : public IPlugin {
public:
    DebuggerVariableViewerPlugin(Debugger* debugger);
    void show() override;

    void refresh();

    int64_t getThreadParent(uint64_t tid);
    std::vector<uint64_t> getThreadIds();
    std::string getThreadIndexVariableName();
    int getThreadIndexVariableValue(uint64_t tid);
    std::vector<Variable> getVariablesInScope(uint sid);
    std::vector<std::vector<Variable>> getVisibleVariables(int pc = -1);

protected:
    Debugger* debugger;
    int watching_thr_id = 0;
};