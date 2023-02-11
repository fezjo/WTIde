#if ZEP_SINGLE_HEADER == 1
#define ZEP_SINGLE_HEADER_BUILD
#endif

#include <clip.h>

#include "config_app.h"
#include "editor_zep_plugin.h"

using namespace Zep;

EditorZepPlugin::EditorZepPlugin(const fs::path &rootPath, const Zep::NVec2f &pixelScale,
                                 std::function<void(std::shared_ptr<Zep::ZepMessage>)> fnCommandCB)
    : editor(Zep::ZepPath(rootPath), pixelScale), Callback(fnCommandCB) {
    static std::unordered_set<std::string> keywords(wt_keywords.begin(), wt_keywords.end());
    static std::unordered_set<std::string> identifiers(wt_identifiers.begin(), wt_identifiers.end());
    editor.RegisterCallback(this);
    editor.RegisterSyntaxFactory({".wt"},
                                 SyntaxProvider{"wt", Zep::tSyntaxFactory([&](ZepBuffer *pBuffer) {
                                                    return std::make_shared<ZepSyntax>(
                                                        *pBuffer, keywords, identifiers);
                                                })});
    pluginType = PluginType::Editor;
    immortal = false;
    dockId = 0;
}

Zep::ZepEditor &EditorZepPlugin::GetEditor() const {
    return static_cast<Zep::ZepEditor &>(const_cast<Zep::ZepEditor_ImGui &>(editor));
}

void EditorZepPlugin::Notify(std::shared_ptr<Zep::ZepMessage> message) {
    if (message->messageId == Msg::GetClipBoard) {
        clip::get_text(message->str);
        message->handled = true;
    } else if (message->messageId == Msg::SetClipBoard) {
        clip::set_text(message->str);
        message->handled = true;
    } else if (message->messageId == Msg::RequestQuit) {
        alive = false;
    } else if (message->messageId == Msg::ToolTip) {
        auto spTipMsg = std::static_pointer_cast<ToolTipMessage>(message);
        if (spTipMsg->location.Valid() && spTipMsg->pBuffer) {
            auto pSyntax = spTipMsg->pBuffer->GetSyntax();
            if (pSyntax) {
                if (pSyntax->GetSyntaxAt(spTipMsg->location).foreground == ThemeColor::Identifier) {
                    auto spMarker = std::make_shared<RangeMarker>(*spTipMsg->pBuffer);
                    spMarker->SetDescription("This is an identifier");
                    spMarker->SetHighlightColor(ThemeColor::Identifier);
                    spMarker->SetTextColor(ThemeColor::Text);
                    spTipMsg->spMarker = spMarker;
                    spTipMsg->handled = true;
                } else if (pSyntax->GetSyntaxAt(spTipMsg->location).foreground ==
                           ThemeColor::Keyword) {
                    auto spMarker = std::make_shared<RangeMarker>(*spTipMsg->pBuffer);
                    spMarker->SetDescription("This is a keyword");
                    spMarker->SetHighlightColor(ThemeColor::Keyword);
                    spMarker->SetTextColor(ThemeColor::Text);
                    spTipMsg->spMarker = spMarker;
                    spTipMsg->handled = true;
                }
            }
        }
    } else
        Callback(message);
}

void EditorZepPlugin::HandleInput() { editor.HandleInput(); }

EditorZepPlugin *EditorZepPlugin::init(const Zep::NVec2f &pixelScale, std::string rootPath) {
    EditorZepPlugin *ep = new EditorZepPlugin(rootPath.empty() ? "." : rootPath,
                                              Zep::NVec2f(pixelScale.x, pixelScale.y),
                                              [](std::shared_ptr<ZepMessage> spMessage) -> void {});

    auto &display = ep->GetEditor().GetDisplay();
    auto pImFont = ImGui::GetIO().Fonts[0].Fonts[0];
    auto pixelHeight = pImFont->FontSize;
    display.SetFont(ZepTextType::UI,
                    std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight)));
    display.SetFont(ZepTextType::Text,
                    std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight)));
    display.SetFont(ZepTextType::Heading1,
                    std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.5)));
    display.SetFont(ZepTextType::Heading2,
                    std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.25)));
    display.SetFont(ZepTextType::Heading3,
                    std::make_shared<ZepFont_ImGui>(display, pImFont, int(pixelHeight * 1.125)));
    return ep;
}

void EditorZepPlugin::update() { GetEditor().RefreshRequired(); }

void EditorZepPlugin::show() {
    if (!shown)
        return;

    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    auto windowClass = ImGuiWindowClass();
    windowClass.DockingAlwaysTabBar = true;
    ImGui::SetNextWindowClass(&windowClass);

    auto &zepBuffer = editor.GetActiveTabWindow()->GetActiveWindow()->GetBuffer();
    title = zepBuffer.GetFilePath().filename();
    if (title.empty())
        title = "Untitled";
    bool dirty = zepBuffer.HasFileFlags(Zep::FileFlags::Dirty);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_UnsavedDocument * dirty; // TODO | ImGuiWindowFlags_NoSavedSettings

    if (!imguiBegin(flags)) {
        ImGui::End();
        return;
    }
    dockId = ImGui::GetWindowDockID();
    editor.isFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
    if (editor.isFocused) {
        lastFocusedTime = get_time();
        editor.HandleInput();
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

    editor.SetDisplayRegion(Zep::NVec2f(min.x, min.y), Zep::NVec2f(max.x, max.y));
    editor.Display();

    ImGui::End();
}

void EditorZepPlugin::setFile(const std::string &filename) {
    getCurrentBuffer().SetFilePath(filename);
    fn = filename;
}

bool EditorZepPlugin::loadFile(const std::string &filename) {
    editor.InitWithFile(filename);
    setFile(filename);
    return true;
}

bool EditorZepPlugin::saveFile(std::string filename) {
    if (filename.empty())
        filename = fn;
    if (fn.empty())
        fn = filename;
    if (filename.empty())
        return false;
    auto &buffer = getCurrentBuffer();
    buffer.SetFilePath(filename);
    editor.SaveBuffer(buffer);
    buffer.SetFilePath(fn);
    return true;
}

ZepBuffer &EditorZepPlugin::getCurrentBuffer() {
    return editor.GetActiveTabWindow()->GetActiveWindow()->GetBuffer();
}