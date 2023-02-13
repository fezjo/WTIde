#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "wt_syntax.h"


struct BreakpointData {
    std::string file;
    int line;
    bool enabled;
    std::string condition;

    BreakpointData(const std::string &file, int line, bool enabled, const std::string &condition)
        : file(file), line(line), enabled(enabled), condition(condition) {}
    
    BreakpointData(const std::string &file, int line)
        : file(file), line(line) {}
};

using bp_callback_t = std::function<void(BreakpointData)>;

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

protected:
    std::string fn;
};