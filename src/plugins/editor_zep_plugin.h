#pragma once

#include <zep.h>

#include "editor_plugin.h"

class EditorZepPlugin : public IEditorPlugin, public Zep::IZepComponent {
public:
    EditorZepPlugin(const fs::path &rootPath, const Zep::NVec2f &pixelScale,
                    std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB);
    virtual Zep::ZepEditor &GetEditor() const override;
    static EditorZepPlugin *init(const Zep::NVec2f &pixelScale, std::string rootPath = "");

    void update() override;
    void show() override;
    bool loadFile(const std::string &filename) override;
    bool saveFile(std::string filename = "") override;

protected:
    virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override;
    virtual void HandleInput();

protected:
    Zep::ZepEditor_ImGui editor;
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;
};