#include <imgui.h>
#include <zep.h>
#include "utils.h"
#include "plugins/plugin.h"
#include "plugins/editor.h"
#include "plugins/filetree.h"

class Window {
    bool initialized = false;
    ImGuiWindowFlags flags;
    std::vector<ZepWrapper*> zepWrappers;
    FileTree *ft;
    std::vector<IPlugin*> plugins; 

    public:
    Window() = default;

    Window& get() {
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

        ft = new FileTree(std::bind(&Window::openEditor, this, std::placeholders::_1));
        ft->displaySize = ImVec2(120, 640);
        ft->setPath(fs::path("."));
        plugins.push_back(ft);
    }

    void update() {
        for(auto p: plugins)
            p->update();
    }

    void show(bool *p_open) {
        // Just show it
        bool use_work_area = true;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
        ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

        if (!ImGui::Begin("Main", p_open, flags)) {
            ImGui::End();
            return;
        }

        for(auto p: plugins) {
            p->show();
            if (!p->alive)
                destroy_plugin(p);
        }
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

    // static void openEditor(Window &w, fs::path path) {
    //     ZepWrapper *zw = ZepWrapper::init(Zep::NVec2f(1.0f, 1.0f));
    //     zw->displaySize = ImVec2(640, 480);
    //     zw->load(Zep::ZepPath(path));
    //     w.zepWrappers.push_back(zw);
    //     w.plugins.push_back(zw);
    // }

    // void openEditor(fs::path path) {
    //     Window::openEditor(*this, path);
    // }

    void openEditor(fs::path path) {
        ZepWrapper *zw = ZepWrapper::init(Zep::NVec2f(1.0f, 1.0f));
        zw->displaySize = ImVec2(640, 480);
        zw->load(Zep::ZepPath(path));
        zepWrappers.push_back(zw);
        plugins.push_back(zw);
    }
};