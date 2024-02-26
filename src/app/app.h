#include <imgui/imgui.h>
#include <zep.h>

#if defined(__EMSCRIPTEN__)
#else
#include <nfd.hpp>
#endif

#include "../imgui/themes.h"
#include "../plugins/plugin.h"
#include "../utils.h"

#include "../plugins/debugger/debugger_control_plugin_v1.h"
#include "../plugins/debugger/debugger_control_plugin_v2.h"
#include "../plugins/debugger/debugger_variable_viewer_plugin.h"
#include "../plugins/debugger/program_analyzer_plugin.h"
#include "../plugins/editor/editor_icte_plugin.h"
#include "../plugins/editor/editor_zep_plugin.h"
#include "../plugins/ide/filetree_plugin.h"
#include "../plugins/ide/plugin_control_plugin.h"
#include "../plugins/text/compiler_output_plugin.h"
#include "../plugins/text/input_plugin.h"
#include "../plugins/text/output_plugin.h"
#include "../plugins/text/text_plugin.h"

#include "../debugger/breakpoint_manager.h"
#include "../debugger/debugger.h"

class App {
public:
    bool alive = false;
    bool show_demo_window = false;
    ImVec4 clear_color;

protected:
#if defined(__EMSCRIPTEN__)
#else
    NFD::Guard nfd_guard;
#endif

    bool initialized = false;
    int untitled_counter = 0;
    std::optional<std::tuple<fs::path, bool, PluginType>> pending_large_file;
    std::vector<IEditorPlugin*> unsaved_dialog_editors;
    timepoint execution_halted_when = timepoint::min();
    ImGuiWindowFlags flags;
    PluginType default_editor_plugin_type = PluginType::EditorIcte;

    Debugger* debugger;
    BreakpointManager breakpoint_storage;
    BreakpointCallbacks breakpoint_callbacks;

    std::vector<IPlugin*> plugins;
    std::vector<IPlugin*> plugins_to_add;
    std::vector<IEditorPlugin*> editor_plugins;
    FileTreePlugin* filetree_plugin;
    PluginControlPlugin* plugin_control_plugin;
    InputPlugin* input_plugin;
    OutputPlugin* output_plugin;
    CompilerOutputPlugin* compiler_output_plugin;
    DebuggerControlPluginV1* debugger_control_plugin_v1;
    DebuggerControlPluginV2* debugger_control_plugin_v2;
    ProgramAnalyzerPlugin* program_analyzer_plugin;
    DebuggerVariableViewerPlugin* debugger_variable_viewer_plugin;

public:
    App() = default;

    void init();
    void switchAppDebugMode();
    void update();
    void show(bool* p_open);
    void quit(bool force = false);

protected:
    void _initializePlugins();
    void _defaultLayout();

    void showDebugWindow();
    void showMainMenuBar();
    void showPendingLargeFile();
    void showNotifications();

    void handleInput();

    void add_plugin(IPlugin* plugin, const std::string& title = "",
                    const ImVec2& size = ImVec2(300, 400));
    void delete_plugin(IPlugin* plugin);

    IEditorPlugin* getLastFocusedEditor(bool updateFocus = false);
    // dockAsLastFocused: if true, the new editor will be opened in the same dockspace as the last
    void openEditor(fs::path path = "", bool dockAsLastFocused = true,
                    PluginType type = PluginType::Unknown, bool force_large_file = false);
    void closeEditor();

    std::vector<fs::path> openMultipleFilesDialog();
    void openFiles();
    fs::path openSaveFileDialog(fs::path defaultPath = "");
    void saveFileAs(std::string path = "", bool rename = true, bool dialog = false);
};
