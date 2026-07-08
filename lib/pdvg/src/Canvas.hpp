/*
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
 */

#pragma once

#include "NanoVG.hpp"
#include "nanovg.h"
#include "Widget.hpp"

#include "Label.hpp"

START_NAMESPACE_DISTRHO

class PDCanvas : public NanoSubWidget
{
public:
    explicit PDCanvas(NanoSubWidget* parent);
    ~PDCanvas();

    void setColors(NVGcolor bgColor);
    void setLabel(std::string text, NVGcolor textColor, int x, int y, int size);
    void updateLabelText(std::string text);
    void setImageData(const unsigned char* data, uint32_t width, uint32_t height);
    void setImageFromMemory(const unsigned char* fileData, uint32_t fileSize);

protected:
    void onNanoDisplay() override;

private:
    NVGcolor bgColor;
    ScopedPointer<PDLabel> label;
    int imageHandle = -1;
    uint32_t imgWidth = 0;
    uint32_t imgHeight = 0;

    DISTRHO_LEAK_DETECTOR(PDCanvas)
};

END_NAMESPACE_DISTRHO
