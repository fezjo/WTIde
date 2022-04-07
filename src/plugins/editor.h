#pragma once

#include <functional>
#include <filesystem>
#include <zep.h>
#include "../utils.h"

namespace fs = std::filesystem;

class ZepWrapper : public Zep::IZepComponent {
    Zep::ZepEditor_ImGui zepEditor;
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;
    uint id;

    public:

    ZepWrapper(
        const fs::path& root_path,
        const Zep::NVec2f& pixelScale,
        std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB,
        uint _id
    );
    virtual Zep::ZepEditor& GetEditor() const override;
    virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override;
    virtual void HandleInput();
    static ZepWrapper* init(const Zep::NVec2f& pixelScale);
    void update();
    void destroy();
    void load(const Zep::ZepPath& file);
    void show(const Zep::NVec2i& displaySize);
    uint getId();
};