#include "app.h"

void App::showDebugWindow() {
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

        ImGui::Text(
            "average of 120 frames\n"
            "%5.1fms=%5.1f FPS\n"
            "last frame\n"
            "%5.1fms=%5.1f FPS",
            1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate,
            1000 * ImGui::GetIO().DeltaTime, 1.0f / ImGui::GetIO().DeltaTime);
        ImGui::End();
    }
}

void App::show(bool *p_open) {
    // Just show it
    bool use_work_area = true;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You
    // can browse its code to learn more about Dear ImGui!).
    if (APP_DEBUG && show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, clear_color);
    if (!ImGui::Begin("Main", p_open, flags)) {
        ImGui::PopStyleColor();
        ImGui::End();
        return;
    }

    showMainMenuBar();

    if (APP_DEBUG)
        showDebugWindow();

    std::vector<IPlugin *> plugins_to_delete;
    for (auto p : plugins) {
        p->show();
        if (!p->alive)
            plugins_to_delete.push_back(p);
    }
    for (auto p : plugins_to_delete)
        delete_plugin(p);

    if (!unsaved_dialog_editors.empty())
        ImGui::OpenPopup("Unsaved changes");
    if (ImGui::BeginPopupModal("Unsaved changes", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("You have unsaved changes.\nDo you want to save them?\n\n");
        ImGui::Separator();

        auto msg = unsaved_dialog_editors.size() > 1
                       ? "Save all"
                       : "Save " + unsaved_dialog_editors[0]->getFileName();
        if (ImGui::Button(msg.c_str(), ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            bool ok = true;
            for (auto p : unsaved_dialog_editors)
                ok &= p->saveFile();
            if (ok)
                quit();
            unsaved_dialog_editors.clear();
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
            unsaved_dialog_editors.clear();
        }
        ImGui::EndPopup();
    }

    showNotifications();

    ImGui::PopStyleColor();
    ImGui::End();
}

void App::showMainMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            auto editor = getLastFocusedEditor();
            if (ImGui::MenuItem("New", "Ctrl+N"))
                openEditor("", true);
            if (ImGui::MenuItem("Open", "Ctrl+O"))
                openFiles();
            if (ImGui::MenuItem("Editor force close", "Ctrl-W", false, editor))
                closeEditor();
            if (ImGui::MenuItem("Save", "Ctrl+S", false, editor && editor->isDirty()))
                saveFileAs("", false);
            if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                saveFileAs("", true, true);
            if (ImGui::MenuItem("Quit"))
                quit();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Appearance")) {
            if (ImGui::BeginMenu("*Font")) {
                // for (auto [name, font] : fonts) {
                //     if (ImGui::MenuItem(name.c_str())) {
                //         ImGui::GetIO().FontDefault = font;
                //         ImGui::GetIO().Fonts->Build();
                //     }
                // }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Theme")) {
                for (auto [name, theme] : THEMES) {
                    if (ImGui::MenuItem(name.c_str())) {
                        ImGui::StyleColorsDark();
                        theme(nullptr);
                        clear_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Plugins")) {
                for (auto p : plugins)
                    ImGui::MenuItem(p->title.c_str(), nullptr, &p->shown);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("DEBUG", nullptr, &APP_DEBUG))
                switchAppDebugMode();
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void App::showNotifications() {
    // Render toasts on top of everything, at the end of your code!
    // You should push style vars here
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f,
                                                    100.f / 255.f)); // Background color
    ImGui::RenderNotifications(); // <-- Here we render all notifications
    ImGui::PopStyleColor(1);
    ImGui::PopStyleVar(1); // Don't forget to Pop()
}