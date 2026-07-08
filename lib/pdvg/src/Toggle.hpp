/*
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
*/

#pragma once

#include "NanoVG.hpp"
#include "nanovg.h"

#include "ExtraEventHandlers.hpp"
#include "Label.hpp"
#include "PDWidget.hpp"

START_NAMESPACE_DISTRHO

class PDToggle : public PDWidget,
               public PDToggleEventHandler
{
public:
    explicit PDToggle(NanoSubWidget* parent, PDToggleEventHandler::Callback* cb);
    ~PDToggle();

    void setColors(NVGcolor bgColor, NVGcolor toggledColor);
    void setLabel(std::string text, NVGcolor textColor, int x, int y, int size);
    void setImageFromMemory(const unsigned char* fileData, uint32_t fileSize);

protected:
    bool onMouse(const MouseEvent &ev) override;
    void onNanoDisplay() override;

private:
    NVGcolor bgColor;
    NVGcolor toggledColor;
    ScopedPointer<PDLabel> label;
    int imageHandle = -1;
    uint32_t imgWidth = 0;
    uint32_t imgHeight = 0;

    DISTRHO_LEAK_DETECTOR(PDToggle)
};

END_NAMESPACE_DISTRHO
