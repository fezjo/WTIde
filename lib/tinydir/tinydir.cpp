// based on https://github.com/cxong/tinydir/blob/9f866c1ec09e62aa4df50d7209556ed705a4dd90/samples/cpp_sample.cpp

#include <iostream>
#include <list>
#include <stdexcept>
#include <string>

#include "tinydir.h"

class TinyDir {
public:
    TinyDir(const std::string& path);
    ~TinyDir();
    std::string baseName() const;
    std::list<tinydir_file> listDir();

private:
    tinydir_dir* dir;
    std::string path;
};


TinyDir::TinyDir(const std::string& path) : path(path), dir(new tinydir_dir) {
    if (tinydir_open(dir, path.c_str()) == -1) {
        throw std::invalid_argument{"path"};
    }
    tinydir_close(dir);
}

TinyDir::~TinyDir() {
    tinydir_close(dir);
    delete dir;
}

std::string TinyDir::baseName() const {
    auto lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos) {
        return path;
    }
    return path.substr(lastSlash + 1);
}

std::list<tinydir_file> TinyDir::listDir()
{
    if (tinydir_open_sorted(dir, path.c_str()) == -1) {
        throw std::invalid_argument{"path"};
    }

    std::list<tinydir_file> files;
    for (int i = 0; i < dir->n_files; i++)
    {
        tinydir_file file;
        tinydir_readfile_n(dir, &file, i);

        files.push_back(file);

        tinydir_next(dir);
    }

    tinydir_close(dir);

    return files;
}