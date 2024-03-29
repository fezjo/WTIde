#include "editor_icte_plugin.h"

EditorIctePlugin::EditorIctePlugin() {
    pluginType = PluginType::EditorIcte;
    immortal = false;
    dockId = 0;

    auto lang = TextEditor_LanguageDefinition_WTStar();
    editor.SetLanguageDefinition(lang);

    bp_handler = BreakpointCallbacks(
        [this](const Breakpoint& bp) {
            if (bp.file != editor.GetPath())
                return false;
            editor.AddBreakpoint(static_cast<int>(bp.line), !bp.condition.empty(), bp.condition,
                                 bp.enabled);
            return true;
        },
        [this](const Breakpoint& bp) {
            if (bp.file != editor.GetPath())
                return false;
            editor.RemoveBreakpoint(static_cast<int>(bp.line));
            return true;
        },
        [this]() {
            auto breakpoints = editor.GetBreakpoints();
            std::set<Breakpoint> res;
            auto file = editor.GetPath();
            for (auto bp : breakpoints)
                res.insert({file, static_cast<uint>(bp.mLine), bp.mEnabled, bp.mCondition});
            return res;
        },
        "handler icte");
}

void EditorIctePlugin::update() {
    auto& io = ImGui::GetIO();
    if (io.KeyMods == ImGuiModFlags_Ctrl && ImGui::IsKeyPressed(ImGuiKey_S))
        saveFile();
}

void EditorIctePlugin::show() {
    if (!shown)
        return;

    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    auto windowClass = ImGuiWindowClass();
    windowClass.DockingAlwaysTabBar = true;
    ImGui::SetNextWindowClass(&windowClass);

    auto filename = fs::path(editor.GetPath()).filename();
    if (!filename.empty())
        title = filename;
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_UnsavedDocument * isDirty(); // TODO | ImGuiWindowFlags_NoSavedSettings
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!imguiBegin(flags)) {
        ImGui::End();
        return;
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("%s", normalize_path(editor.GetPath()).c_str());
    dockId = ImGui::GetWindowDockID();
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
        lastFocusedTime = get_time();

    if (focused && ImGui::BeginMainMenuBar()) {
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

        if (ImGui::BeginMenu("Appearance")) {
            if (ImGui::BeginMenu("ICTE")) {
                static bool showWhitespaces = false;
                if (ImGui::MenuItem("Show whitespace", nullptr, &showWhitespaces))
                    editor.SetShowWhitespaces(showWhitespaces);
                if (ImGui::BeginMenu("Pallette")) {
                    if (ImGui::MenuItem("Dark"))
                        editor.SetPalette(TextEditor::GetDarkPalette());
                    if (ImGui::MenuItem("Light"))
                        editor.SetPalette(TextEditor::GetLightPalette());
                    if (ImGui::MenuItem("Retro blue"))
                        editor.SetPalette(TextEditor::GetRetroBluePalette());
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
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

void EditorIctePlugin::setFile(const std::string& filename) {
    fn = normalize_path(filename, true);
    editor.SetPath(fn);
}

bool EditorIctePlugin::loadFile(const std::string& filename) {
    std::ifstream t(filename);
    if (!t.good())
        return false;
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    editor.SetText(str);
    editor.ResetTextChanged();
    setFile(filename);
    return true;
}

bool EditorIctePlugin::saveFile(std::string filename, bool rename) {
    if (filename.empty())
        filename = fn;
    std::ofstream t(filename, std::ios::out);
    if (!t.good())
        return false;
    t << editor.GetText();
    bool ok = t.good();
    if (ok) {
        editor.ResetTextChanged();
        if (rename)
            setFile(filename);
    }
    return ok;
}

bool EditorIctePlugin::isDirty() const { return editor.IsTextChanged(); }

void EditorIctePlugin::setBreakpointCallbacks(const BreakpointCallbacks& callbacks) {
    editor.OnBreakpointUpdate = [=](TextEditor* te, int line, bool conditioned,
                                    const std::string& condition, bool enabled) {
        callbacks.update({te->GetPath(), static_cast<uint>(line), enabled, condition});
    };
    editor.OnBreakpointRemove = [=](TextEditor* te, int line) {
        callbacks.remove({te->GetPath(), static_cast<uint>(line)});
    };
}

void EditorIctePlugin::setDebuggerLine(size_t line, bool focus) {
    int iline = static_cast<int>(line);
    editor.SetCurrentLineIndicator(iline * focus, false);
    std::vector<int> lines = {iline};
    editor.SetHighlightedLines(lines);
}