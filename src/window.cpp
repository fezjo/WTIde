
#include "imgui.h"

#include <zep.h>
#include "editor.h"

class Window {
    bool initialized = false;
    ImGuiWindowFlags flags;

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
        zep_init(Zep::NVec2f(1.0f, 1.0f));
        zep_load(Zep::ZepPath("..") / "src" / "WTEdu.cpp");
    }

    void update() {
        zep_update();
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
        zep_show(size);
        ImGui::End();

    }

    void destroy() {
        zep_destroy();
    }
};