#pragma once

#include "../utils.h"
#include "text_plugin.h"

class OutputPlugin: public TextPlugin {
public:
    OutputPlugin();

protected:
    using TextPlugin::setReadonly;
};