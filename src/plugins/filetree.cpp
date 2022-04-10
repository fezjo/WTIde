#include "filetree.h"

FileTree::FileTree() {
    pluginType = PT_FileTree;
}

void FileTree::update() {

}

void FileTree::showTree(
    std::string name,
    std::string path,
    ImGuiTreeNodeFlags root_flags,
    ImGuiTreeNodeFlags base_flags,
    uint &node_i,
    std::unordered_set<std::string> &selection
)
{
    std::set<std::pair<bool, fs::path>> entries;
    for(auto &e: fs::directory_iterator(fs::path(path)))
        entries.insert({!e.is_directory(), e.path()});

    if (!node_i)
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNodeEx((void*)(intptr_t)node_i, root_flags, "%s", name.c_str()))
    {
        std::string node_clicked = "";
        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            node_clicked = path;
        for(auto& [is_file, f]: entries)
        {
            std::string f_path = f.string();
            std::string f_name = f.filename();
            node_i++;
            ImGuiTreeNodeFlags node_flags = base_flags;
            if (selection.count(f_path))
                node_flags |= ImGuiTreeNodeFlags_Selected;
            if (!is_file)
                FileTree::showTree(f_name, f_path, node_flags, base_flags, node_i, selection);
            else
            {
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                ImGui::TreeNodeEx((void*)(intptr_t)node_i, node_flags, "%s", f_name.c_str());
            }
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                node_clicked = f_path;
        }
        if (!node_clicked.empty())
        {
            std::cerr << node_clicked << std::endl;
            // Update selection state
            // (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
            if (ImGui::GetIO().KeyCtrl) // CTRL+click to toggle
            {
                if (selection.count(node_clicked))
                    selection.erase(node_clicked);
                else
                    selection.insert(node_clicked);
            }
            else                        // Click to single-select
            {
                selection.clear();
                selection.insert(node_clicked);
            }
        }
        ImGui::TreePop();
    }
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
        ImGui::MenuItem(u8"ﱐ");
        ImGui::MenuItem(u8"");
        ImGui::MenuItem(u8"");
        ImGui::MenuItem(u8"");
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
    
    ImGuiTreeNodeFlags base_flags = 
        ImGuiTreeNodeFlags_SpanAvailWidth;
    uint node_i = 0;
    FileTree::showTree(cwd, cwd, base_flags, base_flags, node_i, selection);

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