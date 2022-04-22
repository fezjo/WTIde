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

class FileTreePlugin: public IPlugin {
public:
    FileTreePlugin();
    void update() override;
    void show() override;
    void destroy() override;

    fs::path getPath();
    fs::path setPath(fs::path path);

    void refresh();

protected:
    void showTree(
        FileTreeNode &node,
        ImGuiTreeNodeFlags base_flags,
        uint &node_i
    );
    
private:
    bool showFileNameActionPopup(); // returns whether popup_string is valid
    bool showFileMenu();
    void handleItem(bool is_dir, fs::path path);
    bool createFile();
    bool createDirectory();
    bool renameFile();
    void openAllSelected();
    void handlePopupActions();

private:
    fs::path node_selected;
    fs::path menu_node;
    std::set<fs::path> selection;
    FileTreeNode root;
    bool files_changed;

    fs::path target_path;
    PopupType popup_type;
    std::string popup_location;
    std::string popup_string;
    ImVec4 popup_color;
};