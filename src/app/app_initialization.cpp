#include "app.h"

void App::init() {
    if (initialized)
        return;
    alive = true;
    initialized = true;
    flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    clear_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

    debugger = new Debugger();
    breakpoint_storage = BreakpointManager(debugger);
    breakpoint_callbacks = BreakpointCallbacks(std::bind(&BreakpointManager::updateBreakpoint, &breakpoint_storage,
                                        std::placeholders::_1),
                            std::bind(&BreakpointManager::removeBreakpoint, &breakpoint_storage,
                                        std::placeholders::_1),
                            std::bind(&BreakpointManager::getBreakpoints, &breakpoint_storage),
                            "root callback", 0);

    _initializePlugins();
    _defaultLayout();
    switchAppDebugMode();
}

void App::_initializePlugins() {
    filetree_plugin = new FileTreePlugin();
    filetree_plugin->setPath(fs::path("."));
    add_plugin(filetree_plugin, "FileTree", ImVec2(120, 640));
    filetree_plugin->setCallback("open_file", [&](CallbackData data) {
        openEditor(std::get<std::string>(data));
        return true;
    });
    filetree_plugin->setCallback("get_editor_icon", [&](CallbackData data) {
        return default_editor_plugin_type == PluginType::EditorIcte ? "ICTE" : "ZEP";
    });
    filetree_plugin->setCallback("switch_default_editor_type", [&](CallbackData data) {
        default_editor_plugin_type = default_editor_plugin_type == PluginType::EditorIcte
                                         ? PluginType::EditorZep
                                         : PluginType::EditorIcte;
        return (int)default_editor_plugin_type;
    });

    plugin_control_plugin = new PluginControlPlugin(&plugins);
    add_plugin(plugin_control_plugin, "Plugins");

    input_plugin = new InputPlugin();
    input_plugin->write("5\n[1 2 3 4 5]\n"); // TODO testing
    add_plugin(input_plugin, "Input");

    output_plugin = new OutputPlugin();
    add_plugin(output_plugin, "Output");

    compiler_output_plugin = new OutputPlugin();
    add_plugin(compiler_output_plugin, "Compiler output");

    debugger_control_plugin_v1 = new DebuggerControlPluginV1(debugger);
    add_plugin(debugger_control_plugin_v1, "Debug Control V1", ImVec2(250, 100));

    debugger_control_plugin_v2 = new DebuggerControlPluginV2(debugger);
    add_plugin(debugger_control_plugin_v2, "Debug Control V2", ImVec2(250, 100));

    std::vector<std::pair<std::string, CallbackFunction>> debugger_control_plugin_callbacks = {
        {"set_input",
         [&](CallbackData data) {
             debugger_control_plugin_v1->setInput(input_plugin->read());
             return true;
         }},
        {"set_output",
         [&](CallbackData data) {
             output_plugin->clear();
             output_plugin->write(std::get<std::string>(data));
             return true;
         }},
        {"set_compilation_output",
         [&](CallbackData data) {
             compiler_output_plugin->write(std::get<std::string>(data));
             compiler_output_plugin->write("\n\n-----\n\n");
             return true;
         }},
        {"refresh_analyzer",
         [&](CallbackData data) {
             program_analyzer_plugin->refresh();
             return true;
         }},
        {"get_focused_source",
         [&](CallbackData data) {
             auto last_focused = getLastFocusedEditor();
             return last_focused ? last_focused->getFileName() : "";
         }},
        {"execution_progress",
         [&](CallbackData data) {
             execution_halted_now = true;
             return true;
         }},
    };
    for (auto [name, callback] : debugger_control_plugin_callbacks) {
        debugger_control_plugin_v1->setCallback(name, callback);
        debugger_control_plugin_v2->setCallback(name, callback);
    }

    program_analyzer_plugin = new ProgramAnalyzerPlugin(debugger);
    add_plugin(program_analyzer_plugin, "Program Analyzer");
    program_analyzer_plugin->bp_callback = breakpoint_callbacks;
    program_analyzer_plugin->bp_callback.info = "analyzer callback";

    debugger_variable_viewer_plugin = new DebuggerVariableViewerPlugin(debugger);
    add_plugin(debugger_variable_viewer_plugin, "Variable Viewer");

    // Called once the fonts/device is guaranteed setup
    openEditor(fs::path("../resources/sample_project/main.cpp"), false, PluginType::EditorZep);
    static_cast<EditorZepPlugin*>(editor_plugins[0])
        ->GetEditor()
        .SetGlobalMode(Zep::ZepMode_Vim::StaticName());

    openEditor(fs::path("../resources/sample_project/test.wt"), false, PluginType::EditorIcte);
}
