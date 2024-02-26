#include "filetree_plugin.h"

bool FileTreePlugin::createFile() {
    if (popup_string.empty())
        return false;
    fs::path create_path = target_path / popup_string;
    if (fs::exists(create_path))
        return false;
    std::fstream creation(create_path, std::fstream::out);
    creation.close();
    return creation.good();
}

bool FileTreePlugin::createDirectory() {
    if (popup_string.empty())
        return false;
    auto create_path = target_path;
    if (!fs::is_directory(create_path))
        create_path = create_path.parent_path();
    create_path /= popup_string;
    if (fs::exists(create_path))
        return false;
    return fs::create_directory(create_path);
}

bool FileTreePlugin::deleteAllSelected() {
    auto todo = selection;
    selection.clear();
    bool ok = true;
    for (auto p : todo)
        if (!fs::remove_all(p))
            ok = false;
    return ok;
}

void FileTreePlugin::openAllSelected() {
    for (auto p : selection) {
        if (fs::is_directory(p))
            continue;
        callbacks["open_file"](p);
    }
}

bool FileTreePlugin::renameFile() {
    if (popup_string.empty())
        return false;
    fs::path create_path = target_path.parent_path() / popup_string;
    if (fs::exists(create_path))
        return false;
    try {
        fs::rename(target_path, create_path);
    } catch (const fs::filesystem_error&) {
        return false;
    }
    return true;
}

void FileTreePlugin::refresh() {
    if (files_changed)
        root.refresh();
    files_changed = false;
}
