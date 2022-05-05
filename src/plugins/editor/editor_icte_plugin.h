#pragma once

#include "TextEditor.h"

#include "editor_plugin.h"

class EditorIctePlugin : public IEditorPlugin {
public:
    EditorIctePlugin();
    virtual void show() override;
    virtual void update() override;
    bool loadFile(const std::string &filename) override;
    bool saveFile(std::string filename = "") override;

protected:
    TextEditor editor;
};