/* 
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
 */

#pragma once

#include <vector>
#include "NanoVG.hpp"
#include "nanovg.h"

#include "PDWidget.hpp"

START_NAMESPACE_DISTRHO

class PDScope : public PDWidget
{
public:
    explicit PDScope(NanoSubWidget* parent);

    void setColors(NVGcolor bgColor, NVGcolor traceColor);
    void pushSamples(const float* samples, uint32_t count);
    void setTriggerLevel(float level);
    void setTriggerMode(bool rising);

protected:
    void onNanoDisplay() override;

private:
    NVGcolor bgColor;
    NVGcolor traceColor;
    std::vector<float> buffer;
    uint32_t writePos = 0;
    uint32_t bufferSize = 512;
    float triggerLevel = 0.0f;
    bool triggerRising = true;
    bool triggered = false;

    DISTRHO_LEAK_DETECTOR(PDScope)
};

END_NAMESPACE_DISTRHO
