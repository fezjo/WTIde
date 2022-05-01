#include "plugin_control_plugin.h"

PluginControlPlugin::PluginControlPlugin(std::vector<IPlugin *> *plugins) : plugins(plugins) {
    pluginType = PluginType::PluginControl;
}

void PluginControlPlugin::showLine(IPlugin *plugin) {
    ImGui::Checkbox(plugin->title.c_str(), &plugin->shown);
    if (!plugin->immortal) {
        ImGui::SameLine();
        if (ImGui::Button(("X##" + std::to_string(plugin->getId())).c_str())) {
            plugin->alive = false;
        }
    }
}

void PluginControlPlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin((title + "###" + std::to_string(getId())).c_str(),
                      immortal ? nullptr : &alive, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return;
    }
    for (auto plugin : *plugins)
        if (isPluginEditor(plugin->getPluginType()))
            showLine(plugin);
    for (auto plugin : *plugins)
        if (!isPluginEditor(plugin->getPluginType()) && plugin != this)
            showLine(plugin);
    ImGui::End();
}