#pragma once

#include <chrono>

#include "text_plugin.h"

enum class CompilerOutputType {
    Fail = -1,
    Info = 0,
    Success = 1,
};

std::string to_string(CompilerOutputType type);

class CompilerOutputPlugin : public TextPlugin {
public:
    CompilerOutputPlugin() {
        pluginType = PluginType::Output;
        setReadonly(true);
    }
    virtual void show() override;
    virtual void clear() override;
    virtual void append(CompilerOutputType type, const std::string& data);
    virtual void write(const std::string& data, size_t start = -1ul) override;
    virtual std::string read(size_t start = 0, size_t size = -1ul) override;

protected:
    using TextPlugin::setReadonly;
    std::string getHeader();

public:
    bool scrollToBottom;

private:
    std::vector<std::tuple<CompilerOutputType, std::string, std::string>> items;
};