#pragma once

#include "../utils.h"
#include "plugin.h"

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
        std::string name,
        std::string path,
        ImGuiTreeNodeFlags root_flags,
        ImGuiTreeNodeFlags base_flags,
        uint &node_i,
        std::unordered_set<std::string> &selection
    );

private:
    fs::path cwd;
    std::unordered_set<std::string> selection;
};