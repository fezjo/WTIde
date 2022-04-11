#if ZEP_SINGLE_HEADER == 1
#define ZEP_SINGLE_HEADER_BUILD
#endif

#include "editor.h"
#include "config_app.h"

using namespace Zep;

using cmdFunc = std::function<void(const std::vector<std::string>&)>; 
class ZepCmd : public ZepExCommand
{
public:
    ZepCmd(ZepEditor& editor, const std::string name, cmdFunc fn)
        : ZepExCommand(editor)
        , m_name(name)
        , m_func(fn)
    {
    }

    virtual void Run(const std::vector<std::string>& args) override
    {
        m_func(args);
    }

    virtual const char* ExCommandName() const override
    {
        return m_name.c_str();
    }

private:
    std::string m_name;
    cmdFunc m_func;
};

ZepWrapper::ZepWrapper(
    const fs::path& _file_path,
    const Zep::NVec2f& pixelScale,
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB,
    imid_t _id
) :
    file_path(_file_path),
    title(_file_path.filename()),
    zepEditor(Zep::ZepPath(_file_path), pixelScale),
    Callback(fnCommandCB)
{
    zepEditor.RegisterCallback(this);
    pluginType = PT_Editor;
}

Zep::ZepEditor& ZepWrapper::GetEditor() const
{
    return (Zep::ZepEditor&)zepEditor;
}

void ZepWrapper::Notify(std::shared_ptr<Zep::ZepMessage> message)
{
    Callback(message);

    return;
}

void ZepWrapper::HandleInput()
{
    zepEditor.HandleInput();
}

ZepWrapper* ZepWrapper::init(const Zep::NVec2f& pixelScale, std::string file)
{
    static uint editor_id = 0;
    ZepWrapper *zw = new ZepWrapper(
        file.empty() ? APP_ROOT : file,
        Zep::NVec2f(pixelScale.x, pixelScale.y),
        [](std::shared_ptr<ZepMessage> spMessage) -> void {},
        editor_id++
    );

    auto& display = zw->GetEditor().GetDisplay();
    auto pImFont = ImGui::GetIO().Fonts[0].Fonts[0];
    auto pixelHeight = pImFont->FontSize;
    display.SetFont(ZepTextType::UI, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight)));
    display.SetFont(ZepTextType::Text, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight)));
    display.SetFont(ZepTextType::Heading1, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.5)));
    display.SetFont(ZepTextType::Heading2, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.25)));
    display.SetFont(ZepTextType::Heading3, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.125)));
    return zw;
}

void ZepWrapper::update()
{
    if(id == 0) {
    GetEditor().RefreshRequired();
    }
}

void ZepWrapper::destroy()
{
}

void ZepWrapper::load(const Zep::ZepPath& file)
{
    auto pBuffer = GetEditor().InitWithFileOrDir(file);
}

void ZepWrapper::show()
{
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(
        (title + "##" + std::to_string(getId())).c_str(),
        &alive,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar
    ))
    {
        ImGui::End();
        return;
    }

    auto min = ImGui::GetCursorScreenPos();
    auto max = ImGui::GetContentRegionAvail();
    if (max.x <= 0)
        max.x = 1;
    if (max.y <= 0)
        max.y = 1;
    ImGui::InvisibleButton("ZepContainer", max);

    // Fill the window
    max.x = min.x + max.x;
    max.y = min.y + max.y;

    zepEditor.SetDisplayRegion(Zep::NVec2f(min.x, min.y), Zep::NVec2f(max.x, max.y));
    zepEditor.isFocused = ImGui::IsWindowFocused();
    zepEditor.Display();
    if (zepEditor.isFocused)
    {
        zepEditor.HandleInput();
    }

    // // TODO: A Better solution for this; I think the audio graph is creating a new window and stealing focus
    // static int focus_count = 0;
    // if (focus_count++ < 2)
    // {
    //     ImGui::SetWindowFocus();
    // }
    ImGui::End();
}