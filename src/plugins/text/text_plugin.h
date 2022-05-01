#pragma once

#include "../../utils.h"
#include "../plugin.h"

class TextPlugin : public IPlugin {
public:
    TextPlugin(bool readonly = false);
    virtual ~TextPlugin() = default;
    virtual void show() override;

    virtual void clear();
    virtual void write(const std::string &data, size_t start = -1);
    virtual std::string read(size_t start = 0, size_t size = -1);
    virtual void setReadonly(bool state);

protected:
    bool readonly;
    std::string data;
};