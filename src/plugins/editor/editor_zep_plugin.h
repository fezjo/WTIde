#pragma once

#include <zep.h>

#include "editor_plugin.h"

class EditorZepPlugin : public IEditorPlugin, public Zep::IZepComponent {
public:
    EditorZepPlugin(const fs::path &rootPath, const Zep::NVec2f &pixelScale,
                    std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB);
    virtual Zep::ZepEditor &GetEditor() const override;
    static EditorZepPlugin *init(const Zep::NVec2f &pixelScale, std::string rootPath = "");
    Zep::ZepBuffer &getCurrentBuffer() const;

    void update() override;
    void show() override;
    void setFile(const std::string &filename) override;
    bool loadFile(const std::string &filename) override;
    bool saveFile(std::string filename = "", bool rename = false) override;
    bool isDirty() const override;

protected:
    virtual void Notify(std::shared_ptr<Zep::ZepMessage> message) override;
    virtual void HandleInput();

protected:
    Zep::ZepEditor_ImGui editor;
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> Callback;

    inline static std::vector<std::string> windowOptions = {
        "ShowWhiteSpace",
        "ShowCR",
        "ShowLineNumbers",
        "ShowIndicators",
        "HideScrollBar",
        "Modal",
        "WrapText",
        "HideSplitMark",
        "GridStyle"
    };
};