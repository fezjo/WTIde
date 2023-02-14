#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "wt_syntax.h"
#include "../../debugger/breakpoint_storage.h"


class IEditorPlugin : public IPlugin {
public:
    virtual ~IEditorPlugin() = default;
    virtual void setFile(const std::string &filename) { fn = filename; };
    virtual bool loadFile(const std::string &filename) = 0;
    virtual bool saveFile(std::string filename = "") = 0;
    virtual bool isDirty() const = 0;
    std::string getFileName() const { return fn; };

    virtual void setBreakpointCallbacks(bp_callback_t update, bp_callback_t remove){};

public:
    timepoint lastFocusedTime;
    ImGuiID dockId;
    BreakpointHandler bpHandler;

protected:
    std::string fn;
};