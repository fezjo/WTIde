#pragma once

#include "../utils.h"
#include "text_plugin.h"

class InputPlugin : public TextPlugin {
public:
    InputPlugin() {
        pluginType = PluginType::Input;
    }

protected:
    using TextPlugin::setReadonly;
};