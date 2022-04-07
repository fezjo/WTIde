#pragma once

#include <imgui.h>
#include "../utils.h"

enum PluginType {
    PT_Editor,
    PT_FileTree,
    PT_Terminal,
    PT_Debugger,
    PT_Output,
    PT_Search
};

class IPlugin {
public:
    virtual void update() = 0;
    virtual void show() = 0;
    virtual void destroy() = 0;

    imid_t getId() { return id; }
    PluginType getPluginType() { return pluginType; }

protected:
    IPlugin() { 
        static imid_t iplugin_id = 0;
        id = iplugin_id++;
    }

public:
    ImVec2 displaySize;

protected:
    imid_t id;
    PluginType pluginType;
};