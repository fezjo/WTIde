#if ZEP_SINGLE_HEADER == 1
#define ZEP_SINGLE_HEADER_BUILD
#endif

#include <clip.h>
#include "editor_plugin.h"
#include "wt_syntax.h"
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

EditorPlugin::EditorPlugin(
    const fs::path& rootPath,
    const Zep::NVec2f& pixelScale,
    std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB
) :
    zepEditor(Zep::ZepPath(rootPath), pixelScale),
    Callback(fnCommandCB),
    dockId(0)
{
    zepEditor.RegisterCallback(this);
    zepEditor.RegisterSyntaxFactory(
        { ".wt" },
        SyntaxProvider{
            "wt", 
            Zep::tSyntaxFactory([](ZepBuffer* pBuffer) {
                return std::make_shared<ZepSyntax>(*pBuffer, wt_keywords, wt_identifiers);
            })
        }
    );
    pluginType = PluginType::Editor;
}

Zep::ZepEditor& EditorPlugin::GetEditor() const
{
    return static_cast<Zep::ZepEditor&>(const_cast<Zep::ZepEditor_ImGui&>(zepEditor));
}

void EditorPlugin::Notify(std::shared_ptr<Zep::ZepMessage> message)
{
    if (message->messageId == Msg::GetClipBoard)
    {
        clip::get_text(message->str);
        message->handled = true;
    }
    else if (message->messageId == Msg::SetClipBoard)
    {
        clip::set_text(message->str);
        message->handled = true;
    }
    else if (message->messageId == Msg::RequestQuit)
    {
        alive = false;
    }
    else if (message->messageId == Msg::ToolTip)
    {
        auto spTipMsg = std::static_pointer_cast<ToolTipMessage>(message);
        if (spTipMsg->location.Valid() && spTipMsg->pBuffer)
        {
            auto pSyntax = spTipMsg->pBuffer->GetSyntax();
            if (pSyntax)
            {
                if (pSyntax->GetSyntaxAt(spTipMsg->location).foreground == ThemeColor::Identifier)
                {
                    auto spMarker = std::make_shared<RangeMarker>(*spTipMsg->pBuffer);
                    spMarker->SetDescription("This is an identifier");
                    spMarker->SetHighlightColor(ThemeColor::Identifier);
                    spMarker->SetTextColor(ThemeColor::Text);
                    spTipMsg->spMarker = spMarker;
                    spTipMsg->handled = true;
                }
                else if (pSyntax->GetSyntaxAt(spTipMsg->location).foreground == ThemeColor::Keyword)
                {
                    auto spMarker = std::make_shared<RangeMarker>(*spTipMsg->pBuffer);
                    spMarker->SetDescription("This is a keyword");
                    spMarker->SetHighlightColor(ThemeColor::Keyword);
                    spMarker->SetTextColor(ThemeColor::Text);
                    spTipMsg->spMarker = spMarker;
                    spTipMsg->handled = true;
                }
            }
        }
    }
    else
        Callback(message);
}

void EditorPlugin::HandleInput()
{
    zepEditor.HandleInput();
}

EditorPlugin* EditorPlugin::init(const Zep::NVec2f& pixelScale, std::string rootPath)
{
    EditorPlugin *ep = new EditorPlugin(
        rootPath.empty() ? APP_ROOT : rootPath,
        Zep::NVec2f(pixelScale.x, pixelScale.y),
        [](std::shared_ptr<ZepMessage> spMessage) -> void {}
    );

    auto& display = ep->GetEditor().GetDisplay();
    auto pImFont = ImGui::GetIO().Fonts[0].Fonts[0];
    auto pixelHeight = pImFont->FontSize;
    display.SetFont(ZepTextType::UI, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight)));
    display.SetFont(ZepTextType::Text, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight)));
    display.SetFont(ZepTextType::Heading1, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.5)));
    display.SetFont(ZepTextType::Heading2, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.25)));
    display.SetFont(ZepTextType::Heading3, std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.125)));
    return ep;
}

void EditorPlugin::update()
{
    GetEditor().RefreshRequired();
}

void EditorPlugin::destroy()
{
}

void EditorPlugin::load(const Zep::ZepPath& file)
{
    GetEditor().InitWithFileOrDir(file);
}

void EditorPlugin::show()
{
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    auto windowClass = ImGuiWindowClass();
    windowClass.DockingAlwaysTabBar = true;
    ImGui::SetNextWindowClass(&windowClass);
    title = zepEditor.GetActiveTabWindow()->GetActiveWindow()->GetBuffer().GetFilePath().filename();
    if (title.empty())
        title = "Untitled";
    if (!ImGui::Begin(
        (title + "###" + std::to_string(getId())).c_str(),
        &alive,
        ImGuiWindowFlags_None // TODO | ImGuiWindowFlags_NoSavedSettings
    ))
    {
        ImGui::End();
        return;
    }
    dockId = ImGui::GetWindowDockID();


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
        lastFocusedTime = get_time();
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