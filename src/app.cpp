#include <imgui/imgui.h>
#include <nfd.hpp>
#include <zep.h>

#include "plugins/plugin.h"
#include "utils.h"

#include "plugins/debugger/program_analyzer_plugin.h"
#include "plugins/editor/editor_icte_plugin.h"
#include "plugins/editor/editor_zep_plugin.h"
#include "plugins/ide/filetree_plugin.h"
#include "plugins/ide/plugin_control_plugin.h"
#include "plugins/text/input_plugin.h"
#include "plugins/text/output_plugin.h"
#include "plugins/text/text_plugin.h"

#include "plugins/debugger/debugger_control_plugin_v1.h" // TODO why does it need to be after zep
#include "plugins/debugger/debugger_control_plugin_v2.h"
#include "plugins/debugger/debugger_variable_viewer_plugin.h"

#include "debugger/debugger.h"
#include "debugger/breakpoint_manager.h"

class App {
public:
    bool alive = false;
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);

protected:
    NFD::Guard nfd_guard;

    bool initialized = false;
    bool show_unsaved_dialog = false;
    bool execution_halted_now = false;
    ImGuiWindowFlags flags;
    PluginType default_editor_plugin_type = PluginType::EditorIcte;

    Debugger *debugger;
    BreakpointManager breakpoint_storage;
    BreakpointCallbacks breakpoint_callbacks;

    std::vector<IPlugin *> plugins;
    std::vector<IEditorPlugin *> editor_plugins;
    FileTreePlugin *filetree_plugin;
    PluginControlPlugin *plugin_control_plugin;
    InputPlugin *input_plugin;
    OutputPlugin *output_plugin;
    OutputPlugin *compiler_output_plugin;
    DebuggerControlPluginV1 *debugger_control_plugin_v1;
    DebuggerControlPluginV2 *debugger_control_plugin_v2;
    ProgramAnalyzerPlugin *program_analyzer_plugin;
    DebuggerVariableViewerPlugin *debugger_variable_viewer_plugin;

public:
    App() = default;

    void init() {
        if (initialized)
            return;
        alive = true;
        initialized = true;
        flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        debugger = new Debugger();
        breakpoint_storage = BreakpointManager(debugger);
        breakpoint_callbacks = {std::bind(&BreakpointManager::updateBreakpoint, &breakpoint_storage,
                                          std::placeholders::_1),
                                std::bind(&BreakpointManager::removeBreakpoint, &breakpoint_storage,
                                          std::placeholders::_1),
                                std::bind(&BreakpointManager::getBreakpoints, &breakpoint_storage),
                                "root callback"};


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
        add_plugin(debugger_control_plugin_v1, "Debug Control", ImVec2(250, 100));
        debugger_control_plugin_v1->setCallback("set_input", [&](CallbackData data) {
            debugger_control_plugin_v1->setInput(input_plugin->read());
            return true;
        });
        debugger_control_plugin_v1->setCallback("set_output", [&](CallbackData data) {
            output_plugin->clear();
            output_plugin->write(std::get<std::string>(data));
            return true;
        });
        debugger_control_plugin_v1->setCallback("set_compilation_output", [&](CallbackData data) {
            compiler_output_plugin->write(std::get<std::string>(data));
            compiler_output_plugin->write("\n\n-----\n\n");
            return true;
        });
        debugger_control_plugin_v1->setCallback("refresh_analyzer", [&](CallbackData data) {
            program_analyzer_plugin->refresh();
            return true;
        });
        debugger_control_plugin_v1->setCallback("get_focused_source", [&](CallbackData data) {
            std::pair<timepoint, IEditorPlugin*> last_focus = {timepoint::min(), nullptr};
            for (auto p : editor_plugins) {
                if (p->lastFocusedTime > last_focus.first)
                    last_focus = {p->lastFocusedTime, p};
            }
            return last_focus.second ? last_focus.second->getFileName() : "";
        });
        debugger_control_plugin_v1->setCallback("execution_progress", [&](CallbackData data) {
            execution_halted_now = true;
            return true;
        });

        debugger_control_plugin_v2 = new DebuggerControlPluginV2(debugger);
        add_plugin(debugger_control_plugin_v2, "Debug Control V2", ImVec2(250, 100));
        debugger_control_plugin_v2->setCallback("set_input", [&](CallbackData data) {
            debugger_control_plugin_v2->setInput(input_plugin->read());
            return true;
        });
        debugger_control_plugin_v2->setCallback("set_output", [&](CallbackData data) {
            output_plugin->clear();
            output_plugin->write(std::get<std::string>(data));
            return true;
        });
        debugger_control_plugin_v2->setCallback("set_compilation_output", [&](CallbackData data) {
            compiler_output_plugin->write(std::get<std::string>(data));
            compiler_output_plugin->write("\n\n-----\n\n");
            return true;
        });
        debugger_control_plugin_v2->setCallback("refresh_analyzer", [&](CallbackData data) {
            program_analyzer_plugin->refresh();
            return true;
        });
        debugger_control_plugin_v2->setCallback("get_focused_source", [&](CallbackData data) {
            std::pair<timepoint, IEditorPlugin*> last_focus = {timepoint::min(), nullptr};
            for (auto p : editor_plugins) {
                if (p->lastFocusedTime > last_focus.first)
                    last_focus = {p->lastFocusedTime, p};
            }
            return last_focus.second ? last_focus.second->getFileName() : "";
        });
        debugger_control_plugin_v2->setCallback("execution_progress", [&](CallbackData data) {
            execution_halted_now = true;
            return true;
        });

        program_analyzer_plugin = new ProgramAnalyzerPlugin(debugger);
        add_plugin(program_analyzer_plugin, "Program Analyzer");
        program_analyzer_plugin->bp_callback = breakpoint_callbacks;
        program_analyzer_plugin->bp_callback.info = "analyzer callback";

        debugger_variable_viewer_plugin = new DebuggerVariableViewerPlugin(debugger);
        add_plugin(debugger_variable_viewer_plugin, "Variable Viewer");

        // Called once the fonts/device is guaranteed setup
        openEditor(fs::path("..") / "src" / "main.cpp", false, PluginType::EditorZep);
        static_cast<EditorZepPlugin *>(editor_plugins[0])
            ->GetEditor()
            .SetGlobalMode(Zep::ZepMode_Vim::StaticName());

        openEditor(fs::path("test.wt"), false, PluginType::EditorIcte);
    }

    void update() {
        handleInput();
        for (auto p : plugins)
            p->update();
        breakpoint_storage.synchronizeHandlers();

        auto [pc, sp] = debugger->getSourcePosition();
        for (auto p : editor_plugins) {
            if (p->getFileName() == sp.file)
                p->setDebuggerLine(sp.line, execution_halted_now);
            else
                p->setDebuggerLine(0);
        }
        execution_halted_now = false;
    }

    void showDebugWindow() {
        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a
        // named window.
        {
            static int counter = 0;

            ImGui::Begin("Debug"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text."); // Display some text (you can use a format
                                                      // strings too)
            ImGui::Checkbox("Demo App",
                            &show_demo_window); // Edit bools storing our window open/close state

            ImGui::ColorEdit3("clear color",
                              (float *)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return
                                         // true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("average of 120 frames\n"
                        "%5.1fms=%5.1f FPS\n"
                        "last frame\n"
                        "%5.1fms=%5.1f FPS",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate,
                        1000 * ImGui::GetIO().DeltaTime, 1.0f / ImGui::GetIO().DeltaTime);
            ImGui::End();
        }
    }

    void show(bool *p_open) {
        // Just show it
        bool use_work_area = true;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You
        // can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, clear_color);
        if (!ImGui::Begin("Main", p_open, flags)) {
            ImGui::PopStyleColor();
            ImGui::End();
            return;
        }

        showDebugWindow();

        std::vector<IPlugin *> plugins_to_delete;
        for (auto p : plugins) {
            p->show();
            if (!p->alive)
                plugins_to_delete.push_back(p);
        }
        for (auto p : plugins_to_delete)
            delete_plugin(p);

        if (show_unsaved_dialog)
            ImGui::OpenPopup("Unsaved changes");
        if (ImGui::BeginPopupModal("Unsaved changes", NULL,
                                   ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::Text("You have unsaved changes.\nDo you want to save them?\n\n");
            ImGui::Separator();

            if (ImGui::Button("Save all", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                for (auto p : editor_plugins)
                    p->saveFile();
                quit();
            }
            ImGui::SameLine();
            if (ImGui::Button("Discard", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                quit(true);
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                show_unsaved_dialog = false;
            }
            ImGui::EndPopup();
        }

        showBreakpointStorage();

        showNotifications();

        ImGui::PopStyleColor();
        ImGui::End();
    }

    void showBreakpointStorage() {
        ImGui::Begin("Breakpoint Storage", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Handlers", ImGuiTreeNodeFlags_None)) {
            for (auto& h : breakpoint_storage.handlers) {
                ImGui::BulletText("%s", h.info.c_str());
            }
        }
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Breakpoints", ImGuiTreeNodeFlags_None)) {
            for (auto& bp : breakpoint_storage.getBreakpoints()) {
                std::stringstream ss;
                ss << bp;
                ImGui::BulletText("%s", ss.str().c_str());
            }
        }
        ImGui::End();
    }

    void showNotifications() {
        // Render toasts on top of everything, at the end of your code!
        // You should push style vars here
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f,
                                                        100.f / 255.f)); // Background color
        ImGui::RenderNotifications(); // <-- Here we render all notifications
        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar(1);        // Don't forget to Pop()
    }

    void handleInput() {
        auto &io = ImGui::GetIO();
        if (io.KeyMods == ImGuiModFlags_Ctrl) {
            if (ImGui::IsKeyPressed(ImGuiKey_N))
                openEditor();
            else if (ImGui::IsKeyPressed(ImGuiKey_O))
                openFiles();
        }
    }

    void quit(bool force = false) {
        if (!force && std::any_of(editor_plugins.begin(), editor_plugins.end(),
                                  [](auto p) { return p->isDirty(); })) {
            show_unsaved_dialog = true;
            return;
        }
        alive = false;
        show_unsaved_dialog = false;
    }

    void add_plugin(IPlugin *plugin, const std::string &title = "",
                    const ImVec2 &size = ImVec2(300, 400)) {
        plugin->title = title;
        plugin->displaySize = size;
        plugins.push_back(plugin);
    }

    void delete_plugin(IPlugin *plugin) {
        if (isPluginEditor(plugin->getPluginType())) {
            editor_plugins.erase(find(editor_plugins.begin(), editor_plugins.end(), plugin));
            if (plugin->getPluginType() == PluginType::EditorIcte)
                breakpoint_storage.removeHandler(static_cast<EditorIctePlugin *>(plugin)->bp_handler.id);
        }
        plugins.erase(find(plugins.begin(), plugins.end(), plugin));
        delete plugin;
    }

    // dockAsLastFocused: if true, the new editor will be opened in the same dockspace as the last
    void openEditor(fs::path path = "", bool dockAsLastFocused = true,
                    PluginType type = PluginType::Unknown) {
        if (type == PluginType::Unknown)
            type = default_editor_plugin_type;
        IEditorPlugin *ep = nullptr;
        if (type == PluginType::EditorIcte) {
            ep = new EditorIctePlugin();
        } else {
            ep = EditorZepPlugin::init(Zep::NVec2f(1.0f, 1.0f));
        }
        if(!ep->loadFile(path))
            ep->setFile(path); // set anyway
        if (type == PluginType::EditorZep) // we can do this only after loadFile which initializes a zep window
            static_cast<EditorZepPlugin *>(ep)->GetEditor().SetGlobalMode(
                Zep::ZepMode_Standard::StaticName());

        ep->setBreakpointCallbacks(breakpoint_callbacks);
        breakpoint_storage.addHandler(ep->bp_handler);
        add_plugin(ep, ep->title, ImVec2(640, 480));
        editor_plugins.push_back(ep);

        if (dockAsLastFocused && !editor_plugins.empty()) {
            std::pair<timepoint, IEditorPlugin *> latest = {editor_plugins[0]->lastFocusedTime,
                                                            nullptr};
            for (IEditorPlugin *epi : editor_plugins) {
                if (epi->lastFocusedTime >= latest.first)
                    latest = {epi->lastFocusedTime, epi};
            }
            ImGui::SetNextWindowDockID(latest.second->dockId);
            ep->show();
        }
    }

    void openFiles() {
        NFD::UniquePathSet outPaths;
        nfdfilteritem_t filterItem[1] = {{"WT* source", "wt"}};
        nfdresult_t result = NFD::OpenDialogMultiple(outPaths, filterItem, 1, ".");
        if (result == NFD_OKAY) {
            nfdpathsetsize_t numPaths;
            NFD::PathSet::Count(outPaths, numPaths);

            nfdpathsetsize_t i;
            for (i = 0; i < numPaths; ++i) {
                NFD::UniquePathSetPath path;
                NFD::PathSet::GetPath(outPaths, i, path);
                openEditor(path.get());
            }
        } else if (result == NFD_CANCEL) {
        } else {
            std::cerr << "Error: " << NFD::GetError() << std::endl;
        }
    }
};
