#pragma once

#include "../../utils.h"
#include "../plugin.h"

struct FileTreeNode {
    bool initialized;
    fs::path path;
    std::vector<FileTreeNode> children;

    FileTreeNode() = default;
    FileTreeNode(fs::path);
    std::vector<FileTreeNode>& listDir();
    void refresh();
};

enum class PopupType { None = 0, NewFile, NewDirectory, Rename, Delete };

class FileTreePlugin : public IPlugin {
public:
    FileTreePlugin();
    void show() override;

    fs::path getPath();
    fs::path setPath(fs::path path);

    void refresh();

private:
    void showTree(FileTreeNode& node, ImGuiTreeNodeFlags base_flags, uint& node_i,
                  int show_type = 1);
    void showMenuBar();
    bool showFileNameActionPopup(); // returns whether popup_string is valid
    void showTreeNodeMenu();
    void showDeleteConfirmation();
    // void showPopup();
    // void showPopup(PopupType type, fs::path location);
    void handleItem(bool is_dir, fs::path path);
    bool createFile();
    bool createDirectory();
    bool deleteAllSelected();
    void openAllSelected();
    bool renameFile();
    void handlePopupActions();

private:
    fs::path node_selected;       // mid frame selected node
    fs::path menu_node;           // node for which the menu is open
    std::set<fs::path> selection; // selected nodes
    FileTreeNode root; // root node of the file tree, initialized with the current working directory
    bool files_changed; // mid frame file changes

    fs::path target_path; // path for single file menu actions (new file, new directory, rename)
    PopupType popup_type;
    std::string popup_location; // path of file for which the popup is open
    std::string popup_string;   // new name for rename, new file, new directory
    ImVec4 popup_color;         // may be red because of error
};