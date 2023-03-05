#pragma once

#include "../../utils.h"
#include "../plugin.h"

class PluginControlPlugin : public IPlugin {
public:
    PluginControlPlugin(std::vector<IPlugin *> *plugins);
    virtual void show() override;

    std::vector<IPlugin *> *plugins;

protected:
    void showLine(IPlugin *plugin);
};