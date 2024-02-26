#include "text_plugin.h"

TextPlugin::TextPlugin(bool readonly) : readonly(readonly) { pluginType = PluginType::Text; }

void TextPlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!imguiBegin(ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return;
    }

    float font_size = ImGui::GetFontSize();
    float max_width = 0;
    {
        data.push_back('\n');
        auto line_begin = data.begin();
        for (auto it = data.begin(); it != data.end(); ++it) {
            if (*it == '\n') {
                float line_width =
                    ImGui::GetFont()
                        ->CalcTextSizeA(font_size, FLT_MAX, 0, &*line_begin, &*it, nullptr)
                        .x;
                max_width = std::max(max_width, line_width);
                line_begin = it;
            }
        }
        data.pop_back();
    }
    float reserve = font_size * 10;
    max_width = std::max(max_width, ImGui::GetWindowWidth()) + reserve;

    ImGuiInputTextFlags flags =
        ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_ReadOnly * readonly;
    ImGui::InputTextMultiline("##data", &data, ImVec2(max_width, -1), flags);

    if (ImGui::BeginPopupContextItem("##menu")) {
        if (ImGui::Selectable("Clear"))
            clear();
        ImGui::EndPopup();
    }

    ImGui::End();
}

void TextPlugin::clear() { data.clear(); }

void TextPlugin::write(const std::string& _data, size_t start) {
    start = std::min(start, data.size());
    size_t new_size = std::max(data.size(), start + _data.size());
    data.resize(new_size);
    _data.copy(data.data() + start, _data.size());
}

std::string TextPlugin::read(size_t start, size_t size) { return data.substr(start, size); }

void TextPlugin::setReadonly(bool state) { readonly = state; }
