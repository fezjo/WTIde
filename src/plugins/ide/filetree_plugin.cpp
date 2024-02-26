#include "filetree_plugin.h"

FileTreeNode::FileTreeNode(fs::path path) : initialized(false), path(path) {}

std::vector<FileTreeNode>& FileTreeNode::listDir() {
    if (!initialized) {
        initialized = true;
        std::set<std::pair<bool, std::string>> entries;
        for (auto& e : fs::directory_iterator(path))
            entries.insert({!e.is_directory(), e.path().filename()});
        children.reserve(entries.size());
        for (auto& [is_file, name] : entries)
            children.push_back(path / name);
    }
    return children;
}

void FileTreeNode::refresh() {
    initialized = false;
    children.clear();
}

FileTreePlugin::FileTreePlugin()
    : files_changed(false), popup_type(PopupType::None), popup_string("") {
    pluginType = PluginType::FileTree;
    title = "FileTree";
}

fs::path FileTreePlugin::getPath() { return root.path; }

fs::path FileTreePlugin::setPath(fs::path path) {
    root.path = path;
    root.refresh();
    return getPath();
}

bool wasNodeSelected() { return ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen(); }

void FileTreePlugin::handleItem(bool is_dir, fs::path path) {
    if (is_dir) {
        if (wasNodeSelected())
            node_selected = path;
    } else if (wasNodeSelected()) {
        if (ImGui::IsMouseDoubleClicked(0)) {
            selection.clear();
            selection.insert(path);
            openAllSelected();
        } else
            node_selected = path;
    }
    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        menu_node = path;
    if (ImGui::IsItemHovered(0, 0.5f))
        ImGui::SetTooltip("%s", normalize_path(path, true).c_str());
    showTreeNodeMenu();
    if (path == popup_location)
        handlePopupActions();
}

void FileTreePlugin::handlePopupActions() {
    if (popup_type == PopupType::None)
        return;
    if (target_path.empty()) {
        if (popup_type == PopupType::Rename) {
            target_path = *selection.begin();
        } else {
            target_path = selection.empty() ? root.path : *selection.begin();
            if (!fs::is_directory(target_path))
                target_path = target_path.parent_path();
        }
        popup_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    }
    if (showFileNameActionPopup()) {
        bool ok = true;
        if (!popup_string.empty()) // if not cancelled
        {
            switch (popup_type) {
            case PopupType::NewFile:
                ok = createFile();
                break;
            case PopupType::NewDirectory:
                ok = createDirectory();
                break;
            case PopupType::Rename:
                ok = renameFile();
                if (ok)
                    selection.clear();
                break;
            default:
                assert(!(bool)"Invalid popup type");
            }
        }
        if (ok) {
            target_path.clear();
            popup_type = PopupType::None;
            popup_location = "";
            popup_string.clear();
            popup_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            files_changed = true;
        } else {
            popup_color = ImVec4(1.0, 0.25, 0.25, 1.0);
        }
    }
}
