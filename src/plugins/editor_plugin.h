#pragma once

#include <functional>
#include <zep.h>
#include "../utils.h"
#include "plugin.h"

class EditorPlugin : public IPlugin, public Zep::IZepComponent {
    Zep::ZepEditor_ImGui zepEditor;
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;

public:
    timepoint lastFocusedTime;
    int dockId;

public:
    EditorPlugin(
        const fs::path& rootPath,
        const Zep::NVec2f& pixelScale,
        std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB
    );
    virtual Zep::ZepEditor& GetEditor() const override;
    virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override;
    virtual void HandleInput();
    void load(const Zep::ZepPath& file);

    static EditorPlugin* init(const Zep::NVec2f& pixelScale, std::string rootPath="");
    void update() override;
    void show() override;
};