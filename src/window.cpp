#include <imgui.h>
#include <zep.h>
#include "utils.h"
#include "editor.h"

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
        zepWrappers.push_back(ZepWrapper::init(Zep::NVec2f(1.0f, 1.0f)));
        zepWrappers.back()->load(Zep::ZepPath("..") / "src" / "WTEdu.cpp");
        zepWrappers.push_back(ZepWrapper::init(Zep::NVec2f(1.0f, 1.0f)));
        zepWrappers.back()->load(Zep::ZepPath("..") / "src" / "editor.cpp");
        zepWrappers.back()->GetEditor().SetGlobalMode(Zep::ZepMode_Standard::StaticName());
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

        static Zep::NVec2i size = Zep::NVec2i(640, 480);
        for(auto zw: zepWrappers)
            zw->show(size);
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