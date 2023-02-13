#pragma once

#include "../../utils.h"
#include "../plugin.h"

#include "wt_syntax.h"

class IEditorPlugin : public IPlugin {
public:
    virtual ~IEditorPlugin() = default;
    virtual void setFile(const std::string &filename) { fn = filename; };
    virtual bool loadFile(const std::string &filename) = 0;
    virtual bool saveFile(std::string filename = "") = 0;
    virtual bool isDirty() const = 0;
    std::string getFileName() const { return fn; };

public:
    timepoint lastFocusedTime;
    ImGuiID dockId;

protected:
    std::string fn;
};