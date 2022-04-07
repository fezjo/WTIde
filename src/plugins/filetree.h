#pragma once

#include "../utils.h"
#include "plugin.h"

class FileTree: IPlugin {
public:
    FileTree();
    void update() override;
    void show() override;
    void destroy() override;
    
    fs::path getPath();
    fs::path setPath();
private:
    fs::path path;
};