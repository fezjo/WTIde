#pragma once

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <imgui_notify.h>

#include "../imgui/imgui_my.h"

#include "../utils.h"

enum class PluginType {
    Unknown = 0,
    FileTree = 1,
    Terminal = 2,
    PluginControl = 3,
    ProgramAnalyzer = 4,
    DebuggerControl = 5,
    Editor = 1 << 7,
    EditorIcte = 1 << 7 | 1,
    EditorZep = 1 << 7 | 2,
    Text = 1 << 8,
    Input = 1 << 8 | 1,
    Output = 1 << 8 | 2,
};

inline bool isPluginEditor(PluginType type) { return (int)type & (int)PluginType::Editor; }

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
    bool shown = true;
    bool immortal = true;
    bool alive = true;
    ImVec2 displaySize;
    std::string title;

protected:
    imid_t id;
    PluginType pluginType;
    std::unordered_map<std::string, CallbackFunction> callbacks;
};