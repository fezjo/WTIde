#pragma once

#include "TextEditor.h"

#include "../utils.h"
#include "plugin.h"

class EditorIctePlugin : public IPlugin {
public:
    EditorIctePlugin();
    virtual void show() override;

public:
    timepoint lastFocusedTime;
    int dockId;

protected:
	TextEditor editor;
    std::string fn;
};