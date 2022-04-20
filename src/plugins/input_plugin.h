#pragma once

#include "../utils.h"
#include "text_plugin.h"

class InputPlugin: public TextPlugin {
public:
    InputPlugin() = default;

protected:
    using TextPlugin::setReadonly;
};