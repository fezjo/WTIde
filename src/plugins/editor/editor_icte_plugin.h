#pragma once

#include "TextEditor.h"

#include "editor_plugin.h"

class EditorIctePlugin : public IEditorPlugin {
public:
    EditorIctePlugin();
    virtual void show() override;
    virtual void update() override;
    void setFile(const std::string& filename) override;
    bool loadFile(const std::string& filename) override;
    bool saveFile(std::string filename = "", bool rename = false) override;
    bool isDirty() const override;

    void setBreakpointCallbacks(const BreakpointCallbacks& handler) override;
    void setDebuggerLine(size_t line, bool focus = false) override;

protected:
    TextEditor editor;
};