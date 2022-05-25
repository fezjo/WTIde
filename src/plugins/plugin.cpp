#include "plugin.h"

std::map<PluginType, std::string> pluginTypeNames = {
    {PluginType::Unknown, "Unknown"},
    {PluginType::FileTree, "FileTree"},
    {PluginType::Terminal, "Terminal"},
    {PluginType::PluginControl, "PluginControl"},

    {PluginType::Text, "Text"},
    {PluginType::Input, "Input"},
    {PluginType::Output, "Output"},

    {PluginType::Editor, "Editor"},
    {PluginType::EditorIcte, "EditorIcte"},
    {PluginType::EditorZep, "EditorZep"},

    {PluginType::DebuggerRelatedPlugin, "DebuggerRelatedPlugin"},
    {PluginType::ProgramAnalyzer, "ProgramAnalyzer"},
    {PluginType::DebuggerControl, "DebuggerControl"},
    {PluginType::DebuggerVariableViewer, "DebuggerVariableViewer"},
};