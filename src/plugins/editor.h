#pragma once

#include <functional>
#include <zep.h>
#include "../utils.h"
#include "plugin.h"

class ZepWrapper : public IPlugin, public Zep::IZepComponent {
    std::string title;
    Zep::ZepEditor_ImGui zepEditor;
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;

public:
    timepoint lastFocusedTime;
    int dockId;

public:
    ZepWrapper(
        const fs::path& rootPath,
        const Zep::NVec2f& pixelScale,
        std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB,
        imid_t _id
    );
    virtual Zep::ZepEditor& GetEditor() const override;
    virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override;
    virtual void HandleInput();
    void load(const Zep::ZepPath& file);

    static ZepWrapper* init(const Zep::NVec2f& pixelScale, std::string rootPath="");
    void update() override;
    void show() override;
    void destroy() override;
};