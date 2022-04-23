#include <imgui.h>
#include <zep.h>
#include "utils.h"
#include "plugins/plugin.h"
#include "plugins/editor_plugin.h"
#include "plugins/filetree_plugin.h"
#include "plugins/text_plugin.h"
#include "plugins/input_plugin.h"
#include "plugins/output_plugin.h"
#include "plugins/debugger_control_plugin.h"

class App {
public:
    bool show_demo_window = false;

protected:
    bool initialized = false;
    ImGuiWindowFlags flags;
    std::vector<EditorPlugin*> editor_plugins;
    FileTreePlugin *filetree_plugin;
    InputPlugin *input_plugin;
    OutputPlugin *output_plugin;
    OutputPlugin *compiler_output_plugin;
    DebuggerControlPlugin *debugger_control_plugin;
    std::vector<IPlugin*> plugins;

public:
    App() = default;

    void init() {
        if (initialized)
            return;
        initialized = true;
        flags = ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings;

        // Called once the fonts/device is guaranteed setup
        openEditor(fs::path("..") / "src" / "WTEdu.cpp", false);
        editor_plugins[0]->GetEditor().SetGlobalMode(Zep::ZepMode_Vim::StaticName());
        openEditor(fs::path("nonexistent.cpp"), false);

        filetree_plugin = new FileTreePlugin();
        filetree_plugin->setCallback("open_file", [&](CallbackData data) {
            openEditor(std::get<std::string>(data), true);
            return true;
        });
        filetree_plugin->displaySize = ImVec2(120, 640);
        filetree_plugin->setPath(fs::path("."));
        plugins.push_back(filetree_plugin);

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
        
        debugger_control_plugin = new DebuggerControlPlugin();
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
            compiler_output_plugin->write("\n\n-----\n\n");
            compiler_output_plugin->write(std::get<std::string>(data));
            return true;
        });
        plugins.push_back(debugger_control_plugin);
    }

    void update() {
        handleInput();
        for(auto p: plugins)
            p->update();
    }

    void show(bool *p_open) {
        static ImVec4 clear_color = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);

        // Just show it
        bool use_work_area = true;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, clear_color);
        if (!ImGui::Begin("Main", p_open, flags)) {
            ImGui::PopStyleColor();
            ImGui::End();
            return;
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static int counter = 0;

            ImGui::Begin("Debug");                                  // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo App", &show_demo_window);      // Edit bools storing our window open/close state

            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text(
                "average of 120 frames\n"
                "%7.3f ms/fr (%5.1f FPS)\n"
                "last frame\n"
                "%7.3fms/fr (%5.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate,
                1000 * ImGui::GetIO().DeltaTime, 1.0f / ImGui::GetIO().DeltaTime
            );
            ImGui::End();
        }

        for(auto p: plugins) {
            p->show();
            if (!p->alive)
                destroy_plugin(p);
        }
        ImGui::PopStyleColor();
        ImGui::End();
    }

    void handleInput() {
        auto& io = ImGui::GetIO();
        if (io.KeyMods == ImGuiKeyModFlags_Ctrl && ImGui::IsKeyPressed(ImGuiKey_N))
        {
            openEditor();
        }
    }

    void destroy_plugin(IPlugin *plugin) {
        if (plugin->getPluginType() == PluginType::Editor)
            editor_plugins.erase(find(editor_plugins.begin(), editor_plugins.end(), plugin));
        plugins.erase(find(plugins.begin(), plugins.end(), plugin));
        plugin->destroy();
        free(plugin);
    }

    void destroy() {
        for (auto it = plugins.rbegin(); it != plugins.rend(); ++it)
            destroy_plugin(*it);
    }

    void openEditor(fs::path path="", bool mimickLastFocused=true) {
        EditorPlugin *ep = EditorPlugin::init(Zep::NVec2f(1.0f, 1.0f));
        ep->displaySize = ImVec2(640, 480);
        ep->GetEditor().SetGlobalMode(Zep::ZepMode_Standard::StaticName());
        if (!path.empty())
            ep->load(Zep::ZepPath(path));
        editor_plugins.push_back(ep);
        plugins.push_back(ep);

        if (mimickLastFocused && !editor_plugins.empty())
        {
            std::pair<timepoint, EditorPlugin*> latest = {editor_plugins[0]->lastFocusedTime, nullptr};
            for (EditorPlugin *epi: editor_plugins)
            {
                if (epi->lastFocusedTime >= latest.first)
                    latest = {epi->lastFocusedTime, epi};
            }
            ImGui::SetNextWindowDockID(latest.second->dockId);
            ep->show();
        }
    }
};
