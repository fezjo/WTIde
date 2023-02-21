#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "wt_syntax.h"
#include "../../debugger/breakpoint_manager.h"


class IEditorPlugin : public IPlugin {
public:
    virtual ~IEditorPlugin() = default;
    virtual void setFile(const std::string &filename) { fn = normalize_path(filename); };
    virtual bool loadFile(const std::string &filename) = 0;
    virtual bool saveFile(std::string filename = "") = 0;
    virtual bool isDirty() const = 0;
    std::string getFileName() const { return fn; };

    virtual void setBreakpointCallbacks(const BreakpointCallbacks &handler) {};
    virtual void setDebuggerLine(size_t line, bool focus = false) {};

public:
    timepoint lastFocusedTime;
    ImGuiID dockId;
    BreakpointCallbacks bp_handler;

protected:
    std::string fn;
};