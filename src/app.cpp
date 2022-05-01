#include <imgui/imgui.h>
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

#include "plugins/debugger/debugger_control_plugin.h" // TODO why does it need to be after zep

#include "debugger/debugger.h"

class App {
public:
    bool show_demo_window = false;
    ImVec4 clear_color = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);

protected:
    bool initialized = false;
    ImGuiWindowFlags flags;
    PluginType default_editor_plugin_type = PluginType::EditorIcte;

    Debugger *debugger;

    std::vector<IPlugin *> plugins;
    std::vector<IEditorPlugin *> editor_plugins;
    FileTreePlugin *filetree_plugin;
    PluginControlPlugin *plugin_control_plugin;
    InputPlugin *input_plugin;
    OutputPlugin *output_plugin;
    OutputPlugin *compiler_output_plugin;
    DebuggerControlPlugin *debugger_control_plugin;
    ProgramAnalyzerPlugin *program_analyzer_plugin;

public:
    App() = default;

    void init() {
        if (initialized)
            return;
        initialized = true;
        flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        debugger = new Debugger();

        // Called once the fonts/device is guaranteed setup
        openEditor(fs::path("..") / "src" / "main.cpp", false, PluginType::EditorZep);
        reinterpret_cast<EditorZepPlugin *>(editor_plugins[0])
            ->GetEditor()
            .SetGlobalMode(Zep::ZepMode_Vim::StaticName());
        openEditor(fs::path("test.wt"), false);

        filetree_plugin = new FileTreePlugin();
        filetree_plugin->setCallback("open_file", [&](CallbackData data) {
            openEditor(std::get<std::string>(data), true, default_editor_plugin_type);
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
        filetree_plugin->displaySize = ImVec2(120, 640);
        filetree_plugin->setPath(fs::path("."));
        plugins.push_back(filetree_plugin);

        plugin_control_plugin = new PluginControlPlugin(&plugins);
        plugin_control_plugin->displaySize = ImVec2(300, 300);
        plugin_control_plugin->title = "Plugins";
        plugins.push_back(plugin_control_plugin);

        TextPlugin *op = new TextPlugin();
        op->displaySize = ImVec2(300, 300);
        op->title = "testing text plugin";
        plugins.push_back(op);

        input_plugin = new InputPlugin();
        input_plugin->displaySize = ImVec2(300, 300);
        input_plugin->title = "Input";
        input_plugin->write("5\n[1 2 3 4 5]\n"); // TODO testing
        plugins.push_back(input_plugin);

        output_plugin = new OutputPlugin();
        output_plugin->displaySize = ImVec2(300, 300);
        output_plugin->title = "Output";
        plugins.push_back(output_plugin);

        compiler_output_plugin = new OutputPlugin();
        compiler_output_plugin->displaySize = ImVec2(300, 300);
        compiler_output_plugin->title = "Compiler output";
        plugins.push_back(compiler_output_plugin);

        debugger_control_plugin = new DebuggerControlPlugin(debugger);
        debugger_control_plugin->displaySize = ImVec2(250, 100);
        debugger_control_plugin->title = "Debug Control";
        debugger_control_plugin->setCallback("set_input", [&](CallbackData data) {
            debugger_control_plugin->setInput(input_plugin->read());
            return true;
        });
        debugger_control_plugin->setCallback("set_output", [&](CallbackData data) {
            output_plugin->clear();
            output_plugin->write(std::get<std::string>(data));
            return true;
        });
        debugger_control_plugin->setCallback("set_compilation_output", [&](CallbackData data) {
            compiler_output_plugin->write(std::get<std::string>(data));
            compiler_output_plugin->write("\n\n-----\n\n");
            return true;
        });
        debugger_control_plugin->setCallback("refresh_analyzer", [&](CallbackData data) {
            program_analyzer_plugin->refresh();
            return true;
        });
        plugins.push_back(debugger_control_plugin);

        program_analyzer_plugin = new ProgramAnalyzerPlugin(debugger);
        program_analyzer_plugin->displaySize = ImVec2(400, 600);
        program_analyzer_plugin->title = "Program Analyzer";
        plugins.push_back(program_analyzer_plugin);
    }

    void update() {
        handleInput();
        for (auto p : plugins)
            p->update();
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
                        "%7.3f ms/fr (%5.1f FPS)\n"
                        "last frame\n"
                        "%7.3fms/fr (%5.1f FPS)",
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
        ImGui::PopStyleColor();

        // Render toasts on top of everything, at the end of your code!
        // You should push style vars here
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f,
                                                        100.f / 255.f)); // Background color
        ImGui::RenderNotifications(); // <-- Here we render all notifications
        ImGui::PopStyleVar(1);        // Don't forget to Pop()
        ImGui::PopStyleColor(1);

        ImGui::End();
    }

    void handleInput() {
        auto &io = ImGui::GetIO();
        if (io.KeyMods == ImGuiKeyModFlags_Ctrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
            openEditor();
        }
    }

    void delete_plugin(IPlugin *plugin) {
        if (isPluginEditor(plugin->getPluginType()))
            editor_plugins.erase(find(editor_plugins.begin(), editor_plugins.end(), plugin));
        plugins.erase(find(plugins.begin(), plugins.end(), plugin));
        delete plugin;
    }

    void openEditor(fs::path path = "", bool mimickLastFocused = true,
                    PluginType type = PluginType::EditorIcte) {
        IEditorPlugin *ep = nullptr;
        if (type == PluginType::EditorIcte) {
            ep = new EditorIctePlugin();
        } else {
            ep = EditorZepPlugin::init(Zep::NVec2f(1.0f, 1.0f));
            reinterpret_cast<EditorZepPlugin *>(ep)->GetEditor().SetGlobalMode(
                Zep::ZepMode_Standard::StaticName());
        }
        ep->displaySize = ImVec2(640, 480);
        if (!path.empty())
            ep->loadFile(path);
        editor_plugins.push_back(ep);
        plugins.push_back(ep);

        if (mimickLastFocused && !editor_plugins.empty()) {
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
};
