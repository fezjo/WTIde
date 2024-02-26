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

    Text = 1 << 6,
    Input = 1 << 6 | 1,
    Output = 1 << 6 | 2,

    Editor = 1 << 7,
    EditorIcte = 1 << 7 | 1,
    EditorZep = 1 << 7 | 2,

    DebuggerRelatedPlugin = 1 << 8,
    ProgramAnalyzer = 1 << 8 | 1,
    DebuggerControl = 1 << 8 | 2,
    DebuggerVariableViewer = 1 << 8 | 3,
};

extern std::map<PluginType, std::string> pluginTypeNames;

inline bool isPluginEditor(PluginType type) { return (int)type & (int)PluginType::Editor; }

using CallbackData = std::variant<bool, int, std::string>;
using CallbackFunction = std::function<CallbackData(CallbackData)>;

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual void update() {}
    virtual void show() = 0;
    virtual void bringToFront() { ImGui::SetWindowFocus(getWindowName().c_str()); }
    virtual void setCallback(const std::string& name, CallbackFunction callback) {
        callbacks[name] = callback;
    }
    virtual void unsetCallback(const std::string& name) { callbacks.erase(name); }

    virtual bool imguiBegin(ImGuiWindowFlags flags = ImGuiWindowFlags_None) {
        return ImGui::Begin(getWindowName().c_str(), immortal ? nullptr : &alive, flags);
    }
    virtual std::string getWindowName() {
        return title + "###plugin_" + pluginTypeNames[pluginType] + "_" + std::to_string(getId());
    }

    imid_t getId() { return id; }
    PluginType getPluginType() { return pluginType; }

protected:
    IPlugin() : id(_iplugin_id++) {}

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

    inline static imid_t _iplugin_id = 0;
};