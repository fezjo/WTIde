#include <imgui.h>
#include <zep.h>
#include "utils.h"
#include "plugins/plugin.h"
#include "plugins/editor.h"
#include "plugins/filetree.h"
#include "plugins/text_plugin.h"
#include "plugins/input.h"
#include "plugins/output.h"

class App {
public:
    bool show_demo_window = false;

protected:
    bool initialized = false;
    ImGuiWindowFlags flags;
    std::vector<ZepWrapper*> zepWrappers;
    FileTree *ft;
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
        openEditor(fs::path("nonexistent.cpp"), false);
        zepWrappers[1]->GetEditor().SetGlobalMode(Zep::ZepMode_Standard::StaticName());

        ft = new FileTree(std::bind(&App::openEditor, this, std::placeholders::_1, true));
        ft->displaySize = ImVec2(120, 640);
        ft->setPath(fs::path("."));
        plugins.push_back(ft);

        TextPlugin *op = new TextPlugin();
        op->displaySize = ImVec2(300, 300);
        op->title = "text plugin";
        plugins.push_back(op);
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
            static float f = 0.0f;
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
            zepWrappers.erase(find(zepWrappers.begin(), zepWrappers.end(), plugin));
        plugins.erase(find(plugins.begin(), plugins.end(), plugin));
        plugin->destroy();
        free(plugin);
    }

    void destroy() {
        for (auto it = plugins.rbegin(); it != plugins.rend(); ++it)
            destroy_plugin(*it);
    }

    void openEditor(fs::path path="", bool mimickLastFocused=true) {
        ZepWrapper *zw = ZepWrapper::init(Zep::NVec2f(1.0f, 1.0f));
        zw->displaySize = ImVec2(640, 480);
        if (!path.empty())
            zw->load(Zep::ZepPath(path));
        zepWrappers.push_back(zw);
        plugins.push_back(zw);

        if (mimickLastFocused && !zepWrappers.empty())
        {
            std::pair<timepoint, ZepWrapper*> latest = {zepWrappers[0]->lastFocusedTime, nullptr};
            for (ZepWrapper *zwi: zepWrappers)
            {
                if (zwi->lastFocusedTime >= latest.first)
                    latest = {zwi->lastFocusedTime, zwi};
            }
            ImGui::SetNextWindowDockID(latest.second->dockId);
            zw->show();
        }
    }
};
