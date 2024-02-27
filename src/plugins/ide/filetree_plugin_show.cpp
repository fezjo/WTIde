#include "filetree_plugin.h"

void FileTreePlugin::showTree(FileTreeNode& node, ImGuiTreeNodeFlags base_flags, uint& node_i,
                              int show_type) {
    ImGuiTreeNodeFlags node_flags = base_flags;
    if (selection.count(node.path))
        node_flags |= ImGuiTreeNodeFlags_Selected;

    bool rootOpen = !show_type;
    if (show_type) {
        if (!node_i)
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        auto title = show_type == 2 ? fs::absolute(node.path).parent_path().filename()
                                    : node.path.filename();
        rootOpen = ImGui::TreeNodeEx((void*)hash_string(node.path.filename()), node_flags, "%s",
                                     title.c_str());
        handleItem(true, node.path);
    }
    if (rootOpen) {
        for (auto& c_node : node.listDir()) {
            node_i++;
            node_flags = base_flags;
            if (fs::is_directory(c_node.path)) {
                FileTreePlugin::showTree(c_node, base_flags, node_i);
            } else {
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (selection.count(c_node.path))
                    node_flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx((void*)(intptr_t)node_i, node_flags, "%s",
                                  c_node.path.filename().c_str());
                handleItem(false, c_node.path);
            }
        }
        if (show_type)
            ImGui::TreePop();
    }
}

void FileTreePlugin::showMenuBar() {
    auto tooltip = [](const std::string_view text) {
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("%s", text.data());
    };
    if (ImGui::BeginMenuBar()) {
        ImGui::BeginDisabled(selection.size() > 1);
        {
            if (ImGui::MenuItem("ﱐ")) {
                ImGui::OpenPopup("filetree_name_action_popup");
                popup_type = PopupType::NewFile;
            }
            tooltip("New file");

            if (ImGui::MenuItem("")) {
                ImGui::OpenPopup("filetree_name_action_popup");
                popup_type = PopupType::NewDirectory;
            }
            tooltip("New directory");

            ImGui::BeginDisabled(selection.size() != 1);
            {
                if (ImGui::MenuItem("Rename")) {
                    ImGui::OpenPopup("filetree_name_action_popup");
                    popup_type = PopupType::Rename;
                }
            }
            tooltip("Rename");

            ImGui::EndDisabled();
        }
        ImGui::EndDisabled();

        if (popup_location == "")
            handlePopupActions();

        ImGui::BeginDisabled(selection.empty());
        {
            if (ImGui::MenuItem(""))
                popup_type = PopupType::Delete;
            tooltip("Delete");
        }
        ImGui::EndDisabled();

        if (ImGui::MenuItem(""))
            files_changed = true;
        tooltip("Refresh");

        std::string editor_icon = std::get<std::string>(callbacks["get_editor_icon"](0));
        if (ImGui::MenuItem(editor_icon.c_str()))
            callbacks["switch_default_editor_type"](0);
        tooltip("Switch default editor");

        ImGui::EndMenuBar();
    }
}

bool FileTreePlugin::showFileNameActionPopup() {
    bool res = false;
    if (ImGui::BeginPopup("filetree_name_action_popup")) {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, popup_color);
        ImGui::SetKeyboardFocusHere(0);
        if (ImGui::InputText("##filetree_create", &popup_string,
                             ImGuiInputTextFlags_EnterReturnsTrue))
            res = true;
        // TODO close popup on escape
        ImGui::PopStyleColor();
        if (ImGui::IsItemEdited())
            popup_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        ImGui::EndPopup();
    } else {
        popup_string.clear();
        res = true;
    }
    return res;
}

void FileTreePlugin::showTreeNodeMenu() {
    if (!ImGui::BeginPopupContextItem())
        return;

    auto openNameActionPopup = [&](PopupType type) {
        popup_type = type;
        target_path = menu_node;
        popup_location = menu_node;
        ImVec2 pos = ImGui::GetWindowPos();
        ImGui::EndPopup();
        ImGui::OpenPopup("filetree_name_action_popup");
        ImGui::SetNextWindowPos(pos);
    };

    if (selection.size() <= 1)
        selection = {menu_node};

    auto label = selection.size() > 1 ? "Open all" : "Open";
    if (ImGui::Selectable(label))
        openAllSelected();

    ImGui::Separator();

    if (ImGui::Selectable("Rename")) {
        openNameActionPopup(PopupType::Rename);
        return;
    }

    label = selection.size() > 1 ? "Delete all" : "Delete";
    if (ImGui::Selectable(label))
        popup_type = PopupType::Delete;

    ImGui::Separator();

    if (ImGui::Selectable("New file")) {
        openNameActionPopup(PopupType::NewFile);
        return;
    }

    if (ImGui::Selectable("New directory")) {
        openNameActionPopup(PopupType::NewDirectory);
        return;
    }

    // TODO copy/cut/paste

    if (ImGui::Selectable("Nothing")) {
    }

    ImGui::EndPopup();
}

void updateSelection(fs::path s, std::set<fs::path>& selection) {
    if (s.empty())
        return;
    if (ImGui::GetIO().KeyCtrl) { // CTRL+click to toggle
        if (selection.count(s))
            selection.erase(s);
        else
            selection.insert(s);
    } else { // Click to single-select
        selection.clear();
        selection.insert(s);
    }
}

void FileTreePlugin::showDeleteConfirmation() {
    if (popup_type == PopupType::Delete) {
        if (selection.empty())
            popup_type = PopupType::None;
        else
            ImGui::OpenPopup("Confirm delete"); // not necessary to call every time
    }
    if (ImGui::BeginPopupModal("Confirm delete", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("Are you sure you want to delete the following files?");
        for (auto p : selection)
            ImGui::Text("%s", p.c_str());
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (!deleteAllSelected())
                ImGui::InsertNotification({ImGuiToastType_Error, 5000,
                                           "Delete failed!\nSome files could not be deleted."});
            ImGui::CloseCurrentPopup();
            popup_type = PopupType::None;
            files_changed = true;
        }
        ImGui::SameLine();
        ImGui::SetItemDefaultFocus();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            popup_type = PopupType::None;
        }
        ImGui::EndPopup();
    }
}

void FileTreePlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!imguiBegin(ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }

    showMenuBar();

    uint node_i = 0;
    FileTreePlugin::showTree(root, ImGuiTreeNodeFlags_SpanAvailWidth, node_i, 2);

    showFileNameActionPopup();
    showDeleteConfirmation();

    refresh();

    // Update selection state
    // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
    updateSelection(node_selected, selection);
    node_selected = "";

    ImGui::End();
}