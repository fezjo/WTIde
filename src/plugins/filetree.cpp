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
    popup_type(0),
    popup_string({})
{
    pluginType = PT_FileTree;
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

void FileTree::showTree(
    FileTreeNode &node,
    ImGuiTreeNodeFlags base_flags,
    uint &node_i
)
{
    ImGuiTreeNodeFlags node_flags = base_flags;
    if (selection.count(node.path))
        node_flags |= ImGuiTreeNodeFlags_Selected;
    fs::path node_clicked;

    if (!node_i)
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    bool rootOpen = ImGui::TreeNodeEx(
        (void*)hash_string(node.path.filename()),
        node_flags,
        "%s",
        node.path.filename().c_str()
    );
    if (wasNodeSelected())
        node_clicked = node.path;
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
                if (wasNodeSelected()) {
                    if (ImGui::IsMouseDoubleClicked(0))
                    {
                        open_callback(c_node.path);
                        selection.clear();
                    }
                    else
                        node_clicked = c_node.path;
                }
            }
        }
        ImGui::TreePop();
    }

    // Update selection state
    // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
    updateSelection(node_clicked, selection);
}

bool FileTree::showCreatePopup(bool new_popup) {
    bool res = false;
    if (ImGui::BeginPopup("filetree_create_popup"))
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
        ImGui::EndMenu();
    }
    else
    {
        popup_string[0] = 0;
        res = true;
    }
    return res;
}

bool FileTree::createFile() {
    if (!popup_string[0])
        return false;
    fs::path create_path = target_path / popup_string.data();
    if (fs::exists(create_path))
        return false;
    if (popup_type == 1)
    {
        std::fstream creation(create_path, std::fstream::out);
        creation.close();
        return creation.good();
    }
    else if (popup_type == 2)
        return fs::create_directory(create_path);
    assert(!"Invalid popup type");
}

void FileTree::show() {
    bool show = true;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(
        ("FileTree##" + std::to_string(getId())).c_str(),
        &show,
        ImGuiWindowFlags_MenuBar
    ))
    {
        ImGui::End();
        return;
    }

    if (ImGui::BeginMenuBar())
    {
        bool changed_files = false;

        ImGui::BeginDisabled(selection.size() > 1);
        {
            int prev_popup_type = popup_type;
            if (ImGui::MenuItem(u8"ﱐ"))
            {
                ImGui::OpenPopup("filetree_create_popup");
                popup_type = 1;
            }
            if (ImGui::MenuItem(u8""))
            {
                ImGui::OpenPopup("filetree_create_popup");
                popup_type = 2;
            }

            bool new_popup = popup_type != prev_popup_type;
            if (new_popup)
            {
                target_path = selection.empty() ? root.path : *selection.begin();
                if (!fs::is_directory(target_path))
                    target_path = target_path.parent_path();
                popup_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
            }
            if (popup_type && showCreatePopup(new_popup))
            {
                if (!popup_string[0] || createFile())
                {
                    popup_type = 0;
                    popup_string[0] = 0;
                    popup_color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
                    changed_files = true;
                }
                else
                {
                    popup_color = ImVec4(1.0, 0.25, 0.25, 1.0);
                }
            }
        }
        ImGui::EndDisabled();
        ImGui::BeginDisabled(selection.empty());
        {
            if (ImGui::MenuItem(u8""))
            {
                for (auto &p: selection)
                    fs::remove_all(p);
                selection.clear();
                changed_files = true;
            }
        }
        ImGui::EndDisabled();
        if (ImGui::MenuItem(u8""))
        {
            changed_files = true;
        }
        ImGui::EndMenuBar();
        
        if (changed_files)
            root.refresh();
    }
    // auto min = ImGui::GetCursorScreenPos();
    // auto max = ImGui::GetContentRegionAvail();
    // if (max.x <= 0)
    //     max.x = 1;
    // if (max.y <= 0)
    //     max.y = 1;
    // ImGui::InvisibleButton("FileTreeContainer", max);

    // // Fill the window
    // max.x = min.x + max.x;
    // max.y = min.y + max.y;
    
    uint node_i = 0;
    FileTree::showTree(root, ImGuiTreeNodeFlags_SpanAvailWidth, node_i);
    
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