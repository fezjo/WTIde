#pragma once

#include <imgui.h>
#include <imgui_stdlib.h>

#include "../utils.h"

enum class PluginType { Unknown = 0, Editor, FileTree, Terminal, Debugger, Output, Search };

using CallbackData = std::variant<bool, int, std::string>;
using CallbackFunction = std::function<CallbackData(CallbackData)>;

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual void update(){};
    virtual void show() = 0;
    virtual void setCallback(const std::string &name, CallbackFunction callback) {
        callbacks[name] = callback;
    }
    virtual void unsetCallback(const std::string &name) { callbacks.erase(name); }

    imid_t getId() { return id; }
    PluginType getPluginType() { return pluginType; }

protected:
    IPlugin() {
        static imid_t iplugin_id = 0;
        id = iplugin_id++;
    }

public:
    bool alive = true;
    ImVec2 displaySize;
    std::string title;

protected:
    imid_t id;
    PluginType pluginType;
    std::unordered_map<std::string, CallbackFunction> callbacks;
};