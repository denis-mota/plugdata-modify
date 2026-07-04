/* 
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
 */

#pragma once

#include "NanoVG.hpp"
#include "nanovg.h"

#include "PDWidget.hpp"
#include "Label.hpp"

START_NAMESPACE_DISTRHO

class PDVU : public PDWidget,
             public IdleCallback
{
public:
    explicit PDVU(NanoSubWidget* parent);

    void setColors(NVGcolor bgColor, NVGcolor fgColor, NVGcolor peakColor);
    void setVertical(bool vertical);
    void setValue(float value);
    void setPeakHoldDuration(uint32_t durationMs);

protected:
    void onNanoDisplay() override;
    void idleCallback() override;

private:
    NVGcolor bgColor;
    NVGcolor fgColor;
    NVGcolor peakColor;
    bool isVertical_ = true;
    float currentValue = 0.0f;
    float peakValue = 0.0f;
    uint32_t peakHoldTime = 0;
    uint32_t peakHoldDurationMs = 1000;
    uint32_t lastIdleTime = 0;

    DISTRHO_LEAK_DETECTOR(PDVU)
};

END_NAMESPACE_DISTRHO
