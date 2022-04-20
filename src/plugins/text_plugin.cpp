#include "text_plugin.h"

TextPlugin::TextPlugin(bool readonly):
    readonly(readonly)
{
}

void TextPlugin::update() {

}

void TextPlugin::show() {
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin(
        (title + "###" + std::to_string(getId())).c_str(),
        nullptr,
        ImGuiWindowFlags_HorizontalScrollbar
    ))
    {
        ImGui::End();
        return;
    }


    float font_size = ImGui::GetFontSize();
    float max_width = 0;
    {
        auto line_begin = data.begin();
        data.push_back('\n');
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            if (*it == '\n')
            {
                float line_width = ImGui::GetFont()->
                    CalcTextSizeA(font_size, FLT_MAX, 0, &*line_begin, &*it, NULL).x;
                max_width = std::max(max_width, line_width);
                line_begin = it;
            }
        }
        data.pop_back();
    }
    float reserve = font_size * 10;
    max_width = std::max(max_width, ImGui::GetWindowWidth()) + reserve;
    
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_ReadOnly * readonly;
    ImGui::InputTextMultiline("##data", &data, ImVec2(max_width, -1), flags);

    ImGui::End();
}

void TextPlugin::destroy() {

}

bool TextPlugin::clear() {
    data.clear();
    return true;
}

std::string TextPlugin::read(size_t start, size_t size) {
    return data.substr(start, size);
}

bool TextPlugin::write(const std::string &_data, size_t start) {
    start = std::min(start, data.size());
    size_t new_size = std::max(data.size(), start + _data.size());
    data.resize(new_size);
    _data.copy(data.data(), _data.size(), start);
    return true;
}

void TextPlugin::setReadonly(bool state) {
    readonly = state;
}
