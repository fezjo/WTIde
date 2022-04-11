#pragma once

#include "../utils.h"
#include "plugin.h"

struct FileTreeNode {
    bool initialized;
    fs::path path;
    std::vector<FileTreeNode> children;

    FileTreeNode() = default;
    FileTreeNode(fs::path);
    std::vector<FileTreeNode>& listDir();
    void refresh();
};

class FileTree: public IPlugin {
public:
    FileTree();
    void update() override;
    void show() override;
    void destroy() override;

    fs::path getPath();
    fs::path setPath(fs::path path);

protected:
    static void showTree(
        FileTreeNode &node,
        ImGuiTreeNodeFlags base_flags,
        uint &node_i,
        std::set<fs::path> &selection
    );
    
private:
    bool showCreatePopup(bool new_popup); // returns whether popup_string is valid
    bool createFile();      // returns whther creation of thing was succesful

private:
    std::set<fs::path> selection;
    FileTreeNode root;

    fs::path target_path;
    int popup_type;
    std::array<char, 64> popup_string;
    ImVec4 popup_color;
};