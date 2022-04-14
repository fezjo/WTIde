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

enum class PopupType {
    None = 0,
    NewFile,
    NewDirectory,
    Rename
};

class FileTree: public IPlugin {
public:
    FileTree() = default;
    FileTree(std::function<void(fs::path)> _open_callback);
    void update() override;
    void show() override;
    void destroy() override;

    fs::path getPath();
    fs::path setPath(fs::path path);

protected:
    void showTree(
        FileTreeNode &node,
        ImGuiTreeNodeFlags base_flags,
        uint &node_i
    );
    
private:
    bool showCreatePopup(bool new_popup); // returns whether popup_string is valid
    bool createFile();      // returns whther creation of thing was succesful
    void openAllSelected();

private:
    std::set<fs::path> selection;
    FileTreeNode root;

    std::function<void(fs::path)> open_callback;

    fs::path target_path;
    PopupType popup_type;
    std::array<char, 64> popup_string;
    ImVec4 popup_color;
};