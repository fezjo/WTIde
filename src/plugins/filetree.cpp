#include "filetree.h"


FileTreeNode::FileTreeNode(fs::path path): path(path), initialized(false) {}

std::vector<FileTreeNode>& FileTreeNode::listDir() {
    if (!initialized)
    {
        initialized = true;
        std::set<std::pair<bool, std::string>> entries;
        for(auto &e: fs::directory_iterator(path))
            entries.insert({!e.is_directory(), e.path().filename()});
        children.reserve(entries.size());
        for(auto &[is_file, name]: entries)
            children.push_back(path / name);
    }
    return children;
}

void FileTreeNode::refresh() {
    initialized = false;
    children.clear();
}

FileTree::FileTree(std::function<void(fs::path)> _open_callback):
    open_callback(_open_callback), 
    popup_type(PopupType::None),
    popup_string({})
{
    pluginType = PluginType::FileTree;
    title = "FileTree";
}

void FileTree::update() {

}

void updateSelection(fs::path s, std::set<fs::path> &selection) {
    if (s.empty())
        return;
    if (ImGui::GetIO().KeyCtrl) // CTRL+click to toggle
    {
        if (selection.count(s))
            selection.erase(s);
        else
            selection.insert(s);
    }
    else                        // Click to single-select
    {
        selection.clear();
        selection.insert(s);
    }
}

bool wasNodeSelected() {
    return ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen();
}

void FileTree::openAllSelected() {
    for(auto p: selection)
        open_callback(p);
}

bool FileTree::showFileMenu() {
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::Selectable("Rename"))
        {
            popup_type = PopupType::Rename;
            target_path = menu_node;
            popup_location = menu_node;
            ImVec2 pos = ImGui::GetWindowPos();
            ImGui::EndPopup();
            ImGui::OpenPopup("filetree_name_action_popup");
            ImGui::SetNextWindowPos(pos);
            return true;
        }

        if (ImGui::Selectable("Nothing"));
        ImGui::EndPopup();
        return true;
    }
    return false;
}

void FileTree::handleItem(bool is_dir, fs::path path) {
    if (is_dir)
    {
        if (wasNodeSelected())
            node_selected = path;
    }
    else
    {
        if (wasNodeSelected()) {
            if (ImGui::IsMouseDoubleClicked(0))
            {
                selection.clear();
                selection.insert(path);
                openAllSelected();
            }
            else
                node_selected = path;
        }
    }
    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        menu_node = path;
    }
    showFileMenu();
    if (path == popup_location)
        handlePopupActions();
}

void FileTree::showTree(
    FileTreeNode &node,
    ImGuiTreeNodeFlags base_flags,
    uint &node_i
)
{
    ImGuiTreeNodeFlags node_flags = base_flags;
    if (selection.count(node.path))
        node_flags |= ImGuiTreeNodeFlags_Selected;

    if (!node_i)
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    bool rootOpen = ImGui::TreeNodeEx(
        (void*)hash_string(node.path.filename()),
        node_flags,
        "%s",
        node.path.filename().c_str()
    );
    handleItem(true, node.path);
    if (rootOpen)
    {
        for(auto &c_node: node.listDir())
        {
            node_i++;
            node_flags = base_flags;
            if (fs::is_directory(c_node.path))
            {
                FileTree::showTree(c_node, base_flags, node_i);
            }
            else
            {
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (selection.count(c_node.path))
                    node_flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx((void*)(intptr_t)node_i, node_flags, "%s", c_node.path.filename().c_str());
                handleItem(false, c_node.path);
                
            }
        }
        ImGui::TreePop();
    }
}

bool FileTree::showFileNameActionPopup() {
    bool res = false;
    if (ImGui::BeginPopup("filetree_name_action_popup"))
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, popup_color);
        ImGui::SetKeyboardFocusHere(0);
        if (ImGui::InputText(
            "##filetree_create",
            popup_string.data(),
            popup_string.max_size(),
            ImGuiInputTextFlags_EnterReturnsTrue
            ))
        {
            res = true;
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemEdited())
            popup_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        ImGui::EndPopup();
    }
    else
    {
        popup_string[0] = 0;
        res = true;
    }
    return res;
}

void FileTree::handlePopupActions() {
    if (popup_type == PopupType::None)
        return;
    if (target_path.empty())
    {
        if (popup_type == PopupType::Rename)
        {
            target_path = *selection.begin();
        }
        else
        {
            target_path = selection.empty() ? root.path : *selection.begin();
            if (!fs::is_directory(target_path))
                target_path = target_path.parent_path();
        }
        popup_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    }
    if (showFileNameActionPopup())
    {
        bool ok = true;
        if (popup_string[0]) // if not cancelled
        {
            switch(popup_type)
            {
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
                    assert(!"Invalid popup type");
            }
        }
        if (ok)
        {
            target_path.clear();
            popup_type = PopupType::None;
            popup_location = "";
            popup_string[0] = 0;
            popup_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            files_changed = true;
        }
        else
        {
            popup_color = ImVec4(1.0, 0.25, 0.25, 1.0);
        }
    }
}

bool FileTree::createFile() {
    if (!popup_string[0])
        return false;
    fs::path create_path = target_path / popup_string.data();
    if (fs::exists(create_path))
        return false;
    std::fstream creation(create_path, std::fstream::out);
    creation.close();
    return creation.good();
}

bool FileTree::createDirectory() {
    if (!popup_string[0])
        return false;
    fs::path create_path = target_path / popup_string.data();
    if (fs::exists(create_path))
        return false;
    return fs::create_directory(create_path);
}

bool FileTree::renameFile() {
    if (!popup_string[0])
        return false;
    fs::path create_path = target_path.parent_path() / popup_string.data();
    if (fs::exists(create_path))
        return false;
    try
    {
        fs::rename(target_path, create_path);
    }
    catch (fs::filesystem_error)
    {
        return false;
    }
    return true;
}

void FileTree::refresh() {
    if (files_changed)
        root.refresh();
    files_changed = false;
}

void FileTree::show() {
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(
        (title + "###" + std::to_string(getId())).c_str(),
        nullptr,
        ImGuiWindowFlags_MenuBar
    ))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        ImGui::BeginDisabled(selection.size() > 1);
        {
            if (ImGui::MenuItem(u8"ﱐ"))
            {
                ImGui::OpenPopup("filetree_name_action_popup");
                popup_type = PopupType::NewFile;
            }
            if (ImGui::MenuItem(u8""))
            {
                ImGui::OpenPopup("filetree_name_action_popup");
                popup_type = PopupType::NewDirectory;
            }
            ImGui::BeginDisabled(selection.size() != 1);
            {
                if (ImGui::MenuItem(u8"Rename"))
                {
                    ImGui::OpenPopup("filetree_name_action_popup");
                    popup_type = PopupType::Rename;
                }
            }
            ImGui::EndDisabled();
        }
        ImGui::EndDisabled();

        if ("" == popup_location)
            handlePopupActions();

        ImGui::BeginDisabled(selection.empty());
        {
            if (ImGui::MenuItem(u8""))
            {
                for (auto &p: selection)
                    fs::remove_all(p);
                selection.clear();
                files_changed = true;
            }
        }
        ImGui::EndDisabled();
        if (ImGui::MenuItem(u8""))
        {
            files_changed = true;
        }
        ImGui::EndMenuBar();

    }

    uint node_i = 0;
    FileTree::showTree(root, ImGuiTreeNodeFlags_SpanAvailWidth, node_i);

    refresh();

    // Update selection state
    // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
    updateSelection(node_selected, selection);
    node_selected = "";
    
    ImGui::End();
}

void FileTree::destroy() {
}

fs::path FileTree::getPath() {
    return root.path;
}

fs::path FileTree::setPath(fs::path path) {
    root.path = path;
    root.refresh();
    return getPath();
}