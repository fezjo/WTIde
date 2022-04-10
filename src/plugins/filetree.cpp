#include "filetree.h"

FileTree::FileTree() {
    pluginType = PT_FileTree;
}

void FileTree::update() {

}

void updateSelection(fs::path s, std::set<fs::path> &selection) {
    if (s.empty())
        return;
    std::cerr << s << std::endl;
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
    fs::path path,
    ImGuiTreeNodeFlags base_flags,
    uint &node_i,
    std::set<fs::path> &selection
)
{
    std::set<std::pair<bool, std::string>> entries;
    for(auto &e: fs::directory_iterator(path))
        entries.insert({!e.is_directory(), e.path().filename()});

    ImGuiTreeNodeFlags node_flags = base_flags;
    if (selection.count(path))
        node_flags |= ImGuiTreeNodeFlags_Selected;
    fs::path node_clicked;

    if (!node_i)
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    bool rootOpen = ImGui::TreeNodeEx((void*)(intptr_t)node_i, node_flags, "%s", path.filename().c_str());
    if (wasNodeSelected())
        node_clicked = path;
    if (rootOpen)
    {
        for(auto& [is_file, name]: entries)
        {
            node_i++;
            fs::path f_path = path / name;
            node_flags = base_flags;
            if (is_file)
            {
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (selection.count(f_path))
                    node_flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx((void*)(intptr_t)node_i, node_flags, "%s", name.c_str());
                if (wasNodeSelected())
                    node_clicked = f_path;
            }
            else
            {
                FileTree::showTree(f_path, base_flags, node_i, selection);
            }
        }
        ImGui::TreePop();
    }

    // Update selection state
    // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
    updateSelection(node_clicked, selection);
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
        ImGui::BeginDisabled(selection.size() > 1);
        {
            fs::path target_path = selection.empty() ? cwd : *selection.begin();
            if (ImGui::MenuItem(u8"ﱐ"))
            {
                std::cerr << "want to create file on " << target_path << std::endl;
            }
            if (ImGui::MenuItem(u8""))
            {
                std::cerr << "want to create directory on " << target_path << std::endl;
                if (!fs::is_directory(target_path))
                    target_path = target_path.parent_path();
                fs::create_directory(target_path / "testdir");
            }
        }
        ImGui::EndDisabled();
        ImGui::BeginDisabled(selection.empty());
        {
            if (ImGui::MenuItem(u8""))
            {
                for (auto &p: selection) {
                    std::cerr << "want to remove file " << p << std::endl;
                    fs::remove_all(p);
                }
                selection.clear();
            }
        }
        ImGui::EndDisabled();
        if (ImGui::MenuItem(u8""))
        {
            std::cerr << "refresh " << std::endl;
        }
        ImGui::EndMenuBar();
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
    FileTree::showTree(cwd, ImGuiTreeNodeFlags_SpanAvailWidth, node_i, selection);
    
    ImGui::End();
}

void FileTree::destroy() {

}

fs::path FileTree::getPath() {
    return cwd;
}

fs::path FileTree::setPath(fs::path path) {
    cwd = path;
    return cwd;
}