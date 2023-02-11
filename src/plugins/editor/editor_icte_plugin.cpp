#include "editor_icte_plugin.h"

EditorIctePlugin::EditorIctePlugin() {
    pluginType = PluginType::EditorIcte;
    immortal = false;
    dockId = 0;

    auto lang = TextEditor_LanguageDefinition_WTStar();
    editor.SetLanguageDefinition(lang);
}

void EditorIctePlugin::update() {
    auto &io = ImGui::GetIO();
    if (io.KeyMods == ImGuiModFlags_Ctrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        saveFile();
    }
}

void EditorIctePlugin::show() {
    if (!shown)
        return;

    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    auto windowClass = ImGuiWindowClass();
    windowClass.DockingAlwaysTabBar = true;
    ImGui::SetNextWindowClass(&windowClass);

    title = fs::path(editor.GetPath()).filename();
    if (title.empty())
        title = "Untitled";
    bool dirty = editor.IsTextChanged();
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_UnsavedDocument * dirty; // TODO | ImGuiWindowFlags_NoSavedSettings
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!imguiBegin(flags)) {
        ImGui::End();
        return;
    }
    dockId = ImGui::GetWindowDockID();
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
        lastFocusedTime = get_time();

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "Ctrl-S")) {
                saveFile();
            }
            if (ImGui::MenuItem("Quit"))
                alive = false;
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            bool ro = editor.IsReadOnly();
            if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                editor.SetReadOnly(ro);
            ImGui::Separator();

            if (ImGui::MenuItem("Undo", "Ctrl-Z", nullptr, !ro && editor.CanUndo()))
                editor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl-Shift-Z", nullptr, !ro && editor.CanRedo()))
                editor.Redo();

            ImGui::Separator();

            if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
                editor.Cut();
            if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                editor.Copy();
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr,
                                !ro && ImGui::GetClipboardText() != nullptr))
                editor.Paste();
            if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
                editor.Delete();

            ImGui::Separator();

            if (ImGui::MenuItem("Select all", "Ctrl-A", nullptr))
                editor.SetSelection(TextEditor::Coordinates(),
                                    TextEditor::Coordinates(editor.GetTotalLines(), 0));

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::MenuItem("Dark palette"))
                editor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
                editor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Retro blue palette"))
                editor.SetPalette(TextEditor::GetRetroBluePalette());
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    auto cpos = editor.GetCursorPosition();
    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1,
                editor.GetTotalLines(), editor.IsOverwrite() ? "Ovr" : "Ins",
                editor.CanUndo() ? "*" : " ", editor.GetLanguageDefinition().mName.c_str(),
                fn.c_str());

    ImGui::PushFont(ImGui::GetFont());
    editor.Render("TextEditor");
    ImGui::PopFont();

    ImGui::End();
}

void EditorIctePlugin::setFile(const std::string &filename) {
    editor.SetPath(filename);
    fn = filename;
}

bool EditorIctePlugin::loadFile(const std::string &filename) {
    std::ifstream t(filename);
    if (!t.good())
        return false;
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    editor.SetText(str);
    editor.ResetTextChanged();
    setFile(filename);
    return true;
}

bool EditorIctePlugin::saveFile(std::string filename) {
    if (filename.empty())
        filename = fn;
    std::ofstream t(filename, std::ios::out);
    if (!t.good())
        return false;
    t << editor.GetText();
    bool ok = t.good();
    if (ok)
        editor.ResetTextChanged();
    return ok;
}