#pragma once

#include "TextEditor.h"

#include "editor_plugin.h"

class EditorIctePlugin : public IEditorPlugin {
public:
    EditorIctePlugin();
    virtual void show() override;
    virtual void update() override;
    void setFile(const std::string &filename) override;
    bool loadFile(const std::string &filename) override;
    bool saveFile(std::string filename = "") override;
    bool isDirty() const override;

    void setBreakpointCallbacks(bp_callback_t update, bp_callback_t remove) override;

protected:
    TextEditor editor;
};