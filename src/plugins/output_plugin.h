#pragma once

#include "../utils.h"
#include "text_plugin.h"

class OutputPlugin : public TextPlugin {
public:
    OutputPlugin() {
        pluginType = PluginType::Output;
        setReadonly(true);
    }

protected:
    using TextPlugin::setReadonly;
};