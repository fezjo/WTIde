#include <imgui.h>
#include <zep.h>
#include "utils.h"
#include "plugins/editor.h"

class Window {
    bool initialized = false;
    ImGuiWindowFlags flags;
    std::vector<ZepWrapper*> zepWrappers;

    public:
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
        ImVec2 size = ImVec2(640, 480);
        for(int i = 0; i < 2; ++i) {
            ZepWrapper *zw = ZepWrapper::init(Zep::NVec2f(1.0f, 1.0f));
            zw->displaySize = size;
            zepWrappers.push_back(zw);
        }

        zepWrappers[0]->load(Zep::ZepPath("..") / "src" / "WTEdu.cpp");
        zepWrappers[1]->load(Zep::ZepPath("..") / "src" / "editor.cpp");
        zepWrappers[1]->GetEditor().SetGlobalMode(Zep::ZepMode_Standard::StaticName());
    }

    void update() {
        for(auto zw: zepWrappers)
            zw->update();
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

        for(auto zw: zepWrappers)
            zw->show();
        ImGui::End();

    }

    void destroy() {
        for(auto zw: zepWrappers) {
            zw->destroy();
            free(zw);
        }
        zepWrappers.clear();
    }
};