/* 
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
 */

#pragma once

#include "NanoVG.hpp"
#include "nanovg.h"

#include "PDWidget.hpp"

START_NAMESPACE_DISTRHO

class PDImage : public PDWidget
{
public:
    explicit PDImage(NanoSubWidget* parent);
    ~PDImage();

    void setBackgroundColor(NVGcolor bgColor);
    void setImageData(const unsigned char* data, uint32_t width, uint32_t height);
    void setBorderColor(NVGcolor borderColor);

protected:
    void onNanoDisplay() override;

private:
    NVGcolor bgColor;
    NVGcolor borderColor_;
    int imageHandle = -1;
    uint32_t imgWidth = 0;
    uint32_t imgHeight = 0;

    DISTRHO_LEAK_DETECTOR(PDImage)
};

END_NAMESPACE_DISTRHO
