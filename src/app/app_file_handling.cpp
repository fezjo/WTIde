#include "app.h"

std::vector<fs::path> App::openMultipleFilesDialog() {
#if defined(__EMSCRIPTEN__)
    return {};
#else
    NFD::UniquePathSet outPaths;
    nfdfilteritem_t filterItem[1] = {{"WT* source", "wt"}};
    nfdresult_t result = NFD::OpenDialogMultiple(outPaths, filterItem, 1, ".");
    if (result == NFD_OKAY) {
        nfdpathsetsize_t numPaths;
        NFD::PathSet::Count(outPaths, numPaths);

        std::vector<fs::path> paths;
        for (nfdpathsetsize_t i = 0; i < numPaths; ++i) {
            NFD::UniquePathSetPath path;
            NFD::PathSet::GetPath(outPaths, i, path);
            paths.push_back(path.get());
        }
        return paths;
    } else if (result == NFD_CANCEL) {
    } else {
        std::cerr << "Error: " << NFD::GetError() << std::endl;
    }
    return {};
#endif
}

void App::openFiles() {
    auto paths = openMultipleFilesDialog();
    for (auto path : paths)
        openEditor(path);
}

fs::path App::openSaveFileDialog(fs::path defaultPath) {
#if defined(__EMSCRIPTEN__)
    return {};
#else
    NFD::UniquePath savePath;
    nfdfilteritem_t filterItem[1] = {{"WT* source", "wt"}};
    fs::path base = defaultPath.empty() ? fs::current_path() : defaultPath.parent_path();
    fs::path name = defaultPath.empty() ? "untitled.wt" : defaultPath.filename();
    nfdresult_t result = NFD::SaveDialog(savePath, filterItem, 1, base.c_str(), name.c_str());
    if (result == NFD_OKAY) {
        return savePath.get();
    } else if (result == NFD_CANCEL) {
        return "";
    } else {
        std::cerr << "Error: " << NFD::GetError() << std::endl;
    }
#endif
}

void App::saveFileAs(std::string path, bool rename, bool dialog) {
    auto editor = getLastFocusedEditor();
    if (!editor)
        return;
    if (path.empty())
        path = editor->getFileName();
    if (dialog)
        path = openSaveFileDialog(path);
    if (!path.empty())
        editor->saveFile(path, rename);
}