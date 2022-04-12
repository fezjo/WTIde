#include <imgui.h>
#include <zep.h>
#include "utils.h"
#include "plugins/plugin.h"
#include "plugins/editor.h"
#include "plugins/filetree.h"

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

    App& get() {
    }

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
        openEditor(fs::path("..") / "src" / "WTEdu.cpp");
        openEditor(fs::path("..") / "src" / "editor.cpp");
        zepWrappers[1]->GetEditor().SetGlobalMode(Zep::ZepMode_Standard::StaticName());

        ft = new FileTree(std::bind(&App::openEditor, this, std::placeholders::_1));
        ft->displaySize = ImVec2(120, 640);
        ft->setPath(fs::path("."));
        plugins.push_back(ft);
    }

    void update() {
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

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

    void destroy_plugin(IPlugin *plugin) {
        if (plugin->getPluginType() == PT_Editor)
            zepWrappers.erase(find(zepWrappers.begin(), zepWrappers.end(), plugin));
        plugins.erase(find(plugins.begin(), plugins.end(), plugin));
        plugin->destroy();
        free(plugin);
    }

    void destroy() {
        for (auto it = plugins.rbegin(); it != plugins.rend(); ++it)
            destroy_plugin(*it);
    }

    void openEditor(fs::path path) {
        ZepWrapper *zw = ZepWrapper::init(Zep::NVec2f(1.0f, 1.0f), path);
        zw->displaySize = ImVec2(640, 480);
        zw->load(Zep::ZepPath(path));
        zepWrappers.push_back(zw);
        plugins.push_back(zw);
    }
};