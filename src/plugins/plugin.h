#pragma once

#include "../utils.h"

enum PluginType {
    Editor,
    FileTree,
    Terminal,
    Debugger,
    Output,
    Search
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