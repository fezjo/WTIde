#include "compiler_output_plugin.h"

std::string to_string(CompilerOutputType type) {
    switch (type) {
    case CompilerOutputType::Fail:
        return "[Fail]";
    case CompilerOutputType::Info:
        return "[Info]";
    case CompilerOutputType::Success:
        return "[Success]";
    default:
        return "";
    }
}

std::optional<ImVec4> get_color(CompilerOutputType type) {
    switch (type) {
    case CompilerOutputType::Fail:
        return ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
    case CompilerOutputType::Info:
        return ImVec4(0.4f, 0.4f, 1.0f, 1.0f);
    case CompilerOutputType::Success:
        return ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
    default:
        return std::nullopt;
    }
}

void CompilerOutputPlugin::show() {
    if (!shown)
        return;
    ImGui::SetNextWindowSize(displaySize, ImGuiCond_FirstUseEver);
    if (!imguiBegin(ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return;
    }

    // based on imgui_demo.cpp console example
    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), ImGuiChildFlags_None,
                          ImGuiWindowFlags_HorizontalScrollbar)) {
        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::Selectable("Clear"))
                clear();
            ImGui::EndPopup();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
        for (auto& item : items) {
            auto [output_type, header, text] = item;
            header += " " + to_string(output_type);
            auto color = get_color(output_type);

            if (color.has_value())
                ImGui::PushStyleColor(ImGuiCol_Text, color.value());
            ImGui::SeparatorText(header.data());
            ImGui::TextUnformatted(text.data());
            if (color.has_value())
                ImGui::PopStyleColor();
        }
        ImGui::PopStyleVar();

        // scrollToBottom is true if an outside function requested scrolling to the bottom
        // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
        // Using a scrollbar or mouse-wheel will take away from the bottom edge.
        if (scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        scrollToBottom = false;
    }
    ImGui::EndChild();
    ImGui::End();
}

void CompilerOutputPlugin::clear() {
    items.clear();
    data.clear();
}

void CompilerOutputPlugin::append(CompilerOutputType type, const std::string& data) {
    items.push_back({type, getHeader(), data});
}

void CompilerOutputPlugin::write(const std::string& data, size_t start) {
    items.push_back({CompilerOutputType::Info, getHeader(), data});
}

std::string CompilerOutputPlugin::read(size_t start, size_t size) {
    std::string ret;
    for (size_t i = start; i < items.size() && i < start + size; i++)
        ret += get<1>(items[i]) + "\n" + get<2>(items[i]) + "\n";
    return ret;
}

std::string CompilerOutputPlugin::getHeader() {
    // return std::string of current time in HH:MM:SS
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
    return ss.str();
}