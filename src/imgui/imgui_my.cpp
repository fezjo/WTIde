#include "imgui_my.h"
#include "imgui_internal.h"

namespace ImGui {
bool IsItemHovered(ImGuiHoveredFlags flags, float timer) {
    return GImGui->HoveredIdTimer >= timer && IsItemHovered(flags);
}
} // namespace ImGui