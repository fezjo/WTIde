#include "filetree.h"

FileTree::FileTree() {
    pluginType = PT_FileTree;
}

void FileTree::update() {

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

    auto min = ImGui::GetCursorScreenPos();
    auto max = ImGui::GetContentRegionAvail();
    if (max.x <= 0)
        max.x = 1;
    if (max.y <= 0)
        max.y = 1;
    ImGui::InvisibleButton("FileTreeContainer", max);

    // Fill the window
    max.x = min.x + max.x;
    max.y = min.y + max.y;

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