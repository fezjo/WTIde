#pragma once

#include <functional>
#include <zep.h>
#include "../utils.h"
#include "plugin.h"

class ZepWrapper : public IPlugin, public Zep::IZepComponent {
    std::string title;
    fs::path file_path;
    Zep::ZepEditor_ImGui zepEditor;
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;

public:
    ZepWrapper(
        const fs::path& _file_path,
        const Zep::NVec2f& pixelScale,
        std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB,
        imid_t _id
    );
    virtual Zep::ZepEditor& GetEditor() const override;
    virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override;
    virtual void HandleInput();
    static ZepWrapper* init(const Zep::NVec2f& pixelScale, std::string file="");
    void load(const Zep::ZepPath& file);
    void update() override;
    void destroy() override;
    void show() override;
};