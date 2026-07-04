/* 
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
 */

#include "nanovg.h"
#include "extra/Time.hpp"

#include "Common.hpp"
#include "VU.hpp"

START_NAMESPACE_DISTRHO

PDVU::PDVU(NanoSubWidget* parent)
    : PDWidget(parent)
{
    getWindow().addIdleCallback(this);
    lastIdleTime = d_gettime_ms();
    peakHoldTime = lastIdleTime;
}

void PDVU::onNanoDisplay()
{
    const float scaleFactor = getTopLevelWidget()->getScaleFactor();
    const DGL::Rectangle<float> b(0.0f, 0.0f, getWidth(), getHeight());
    NVGcontext* nvg = getContext();

    // Draw background
    drawRoundedRect(nvg, b.getX(), b.getY(), b.getWidth(), b.getHeight(),
                    bgColor, Colors::outColor, Corners::objectCornerRadius * scaleFactor);

    // Draw meter bar
    const DGL::Rectangle<float> bar = reduceRectangle(b, 1.5f * scaleFactor);
    const float norm = jmax(0.0f, jmin(1.0f, currentValue));

    // Segment colors: green -> yellow -> red
    NVGcolor barColor;
    if (norm < 0.6f)
        barColor = fgColor;
    else if (norm < 0.85f)
        barColor = interpolateColors(fgColor, nvgRGBA(255, 200, 0, 255), (norm - 0.6f) / 0.25f);
    else
        barColor = interpolateColors(nvgRGBA(255, 200, 0, 255), nvgRGBA(255, 50, 50, 255), (norm - 0.85f) / 0.15f);

    if (isVertical_)
    {
        const float fillH = bar.getHeight() * norm;
        const float fillY = bar.getY() + bar.getHeight() - fillH;
        drawRoundedRect(nvg, bar.getX(), fillY, bar.getWidth(), fillH,
                        barColor, barColor, 1.0f);

        // Peak hold indicator
        if (peakValue > 0.01f)
        {
            const float peakH = bar.getHeight() * peakValue;
            const float peakY = bar.getY() + bar.getHeight() - peakH;
            nvgBeginPath(nvg);
            nvgRect(nvg, bar.getX(), peakY - 2.0f * scaleFactor, bar.getWidth(), 2.0f * scaleFactor);
            nvgFillColor(nvg, peakColor);
            nvgFill(nvg);
        }
    }
    else
    {
        const float fillW = bar.getWidth() * norm;
        drawRoundedRect(nvg, bar.getX(), bar.getY(), fillW, bar.getHeight(),
                        barColor, barColor, 1.0f);

        // Peak hold indicator
        if (peakValue > 0.01f)
        {
            const float peakW = bar.getWidth() * peakValue;
            nvgBeginPath(nvg);
            nvgRect(nvg, bar.getX() + peakW, bar.getY(), 2.0f * scaleFactor, bar.getHeight());
            nvgFillColor(nvg, peakColor);
            nvgFill(nvg);
        }
    }

    // Draw grid lines
    NVGcolor gridColor = nvgRGBA(0x62, 0x62, 0x62, 76);
    nvgStrokeColor(nvg, gridColor);
    nvgStrokeWidth(nvg, 0.5f * scaleFactor);
    for (int i = 1; i < 10; ++i)
    {
        float t = i / 10.0f;
        nvgBeginPath(nvg);
        if (isVertical_)
        {
            float y = bar.getY() + bar.getHeight() * (1.0f - t);
            nvgMoveTo(nvg, bar.getX(), y);
            nvgLineTo(nvg, bar.getX() + 3.0f * scaleFactor, y);
        }
        else
        {
            float x = bar.getX() + bar.getWidth() * t;
            nvgMoveTo(nvg, x, bar.getY());
            nvgLineTo(nvg, x, bar.getY() + 3.0f * scaleFactor);
        }
        nvgStroke(nvg);
    }
}

void PDVU::idleCallback()
{
    const uint32_t now = d_gettime_ms();
    const uint32_t elapsed = now - peakHoldTime;

    // Peak hold with time-based decay: 2 seconds from peak to zero
    if (peakValue > currentValue && elapsed > peakHoldDurationMs)
    {
        float decayPerMs = 1.0f / 2000.0f; // full decay in 2000ms
        peakValue -= decayPerMs * deltaMs;
        if (peakValue < currentValue)
            peakValue = currentValue;
        repaint();
    }
    lastIdleTime = now;

    // Track peak
    if (currentValue > peakValue)
    {
        peakValue = currentValue;
        peakHoldTime = now;
    }
}

void PDVU::setValue(float value)
{
    currentValue = jmax(0.0f, jmin(1.0f, value));
    repaint();
}

void PDVU::setVertical(bool vertical)
{
    isVertical_ = vertical;
    repaint();
}

void PDVU::setColors(NVGcolor bgColor, NVGcolor fgColor, NVGcolor peakColor)
{
    this->bgColor = bgColor;
    this->fgColor = fgColor;
    this->peakColor = peakColor;
}

void PDVU::setPeakHoldDuration(uint32_t durationMs)
{
    peakHoldDurationMs = durationMs;
}

END_NAMESPACE_DISTRHO
