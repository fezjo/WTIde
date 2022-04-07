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
private:
    fs::path cwd;
};