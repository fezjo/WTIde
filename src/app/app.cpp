#include "app.h"

void App::switchAppDebugMode() {
    plugin_control_plugin->shown = APP_DEBUG;
    debugger_control_plugin_v1->shown = APP_DEBUG;
    program_analyzer_plugin->shown = APP_DEBUG;
}

void App::update() {
    handleInput();
    for (auto p : plugins)
        p->update();
    breakpoint_storage.synchronizeHandlers();

    auto [pc, sp] = debugger->getSourcePosition();

    auto halt_millis =
        std::chrono::duration_cast<std::chrono::milliseconds>(get_time() - execution_halted_when);
    if (halt_millis.count() > 500)
        execution_halted_when = timepoint::min();
    bool execution_halted_now = execution_halted_when != timepoint::min();
    for (auto p : editor_plugins)
        if (p->getFileName() == sp.file)
            p->setDebuggerLine(sp.line, execution_halted_now);
        else
            p->setDebuggerLine(0);

    getLastFocusedEditor(true);
}

void App::handleInput() {
    auto& io = ImGui::GetIO();
    if (io.KeyMods == ImGuiModFlags_Ctrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_N))
            openEditor();
        else if (ImGui::IsKeyPressed(ImGuiKey_O))
            openFiles();
        else if (ImGui::IsKeyPressed(ImGuiKey_S))
            saveFileAs("", false);
        else if (ImGui::IsKeyPressed(ImGuiKey_W))
            closeEditor();
    }
    if (io.KeyMods == (ImGuiModFlags_Ctrl | ImGuiModFlags_Shift)) {
        if (ImGui::IsKeyPressed(ImGuiKey_S))
            saveFileAs("", true, true);
    }
}

void App::quit(bool force) {
    if (!force) {
        unsaved_dialog_editors.clear();
        for (auto p : editor_plugins)
            if (p->isDirty())
                unsaved_dialog_editors.push_back(p);
        dbg(unsaved_dialog_editors.size());
        if (!unsaved_dialog_editors.empty())
            return;
    }
    alive = false;
    unsaved_dialog_editors.clear();
}

void App::add_plugin(IPlugin* plugin, const std::string& title, const ImVec2& size) {
    plugin->title = title;
    plugin->displaySize = size;
    plugins.push_back(plugin);
}

void App::delete_plugin(IPlugin* plugin) {
    if (isPluginEditor(plugin->getPluginType())) {
        editor_plugins.erase(find(editor_plugins.begin(), editor_plugins.end(), plugin));
        if (plugin->getPluginType() == PluginType::EditorIcte)
            breakpoint_storage.removeHandler(static_cast<EditorIctePlugin*>(plugin)->bp_handler.id);
    }
    plugins.erase(find(plugins.begin(), plugins.end(), plugin));
    delete plugin;
}

IEditorPlugin* App::getLastFocusedEditor(bool updateFocus) {
    std::pair<timepoint, IEditorPlugin*> res = {timepoint::min(), nullptr};
    for (auto editor : editor_plugins) {
        if (updateFocus)
            editor->focused = false;
        if (editor->lastFocusedTime > res.first)
            res = {editor->lastFocusedTime, editor};
    }
    if (updateFocus)
        res.second->focused = true;
    return res.second;
}

void App::openEditor(fs::path path, bool dockAsLastFocused, PluginType type) {
    if (type == PluginType::Unknown)
        type = default_editor_plugin_type;
    IEditorPlugin* ep = nullptr;
    if (type == PluginType::EditorIcte)
        ep = new EditorIctePlugin();
    else
        ep = EditorZepPlugin::init(Zep::NVec2f(1.0f, 1.0f));

    if (!ep->loadFile(path))
        ep->setFile(path); // set anyway
    if (ep->getFileName().empty()) {
        while (true) {
            fs::path untitled_path = fs::path("untitled-" + std::to_string(untitled_counter++));
            if (fs::exists(untitled_path))
                continue;
            ep->title = untitled_path;
            break;
        }
    }

    // we can do this only after loadFile which initializes a zep window
    if (type == PluginType::EditorZep)
        static_cast<EditorZepPlugin*>(ep)->GetEditor().SetGlobalMode(
            Zep::ZepMode_Standard::StaticName());

    ep->setBreakpointCallbacks(breakpoint_callbacks);
    breakpoint_storage.addHandler(ep->bp_handler);
    add_plugin(ep, ep->title, ImVec2(640, 480));
    editor_plugins.push_back(ep);

    if (dockAsLastFocused && !editor_plugins.empty()) {
        ImGui::SetNextWindowDockID(getLastFocusedEditor()->dockId);
        ep->show();
    }
}

void App::closeEditor() {
    auto editor = getLastFocusedEditor();
    if (!editor)
        return;
    if (editor->isDirty()) {
        unsaved_dialog_editors.push_back(editor);
        return;
    }
    delete_plugin(editor);
}