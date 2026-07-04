/* 
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
 */

#include "nanovg.h"

#include "Common.hpp"
#include "Scope.hpp"

START_NAMESPACE_DISTRHO

PDScope::PDScope(NanoSubWidget* parent)
    : PDWidget(parent),
      buffer(bufferSize, 0.0f)
{
}

void PDScope::pushSamples(const float* samples, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        buffer[writePos] = samples[i];
        writePos = (writePos + 1) % bufferSize;

        // Simple trigger detection
        if (!triggered)
        {
            uint32_t prev = (writePos + bufferSize - 1) % bufferSize;
            if (triggerRising)
                triggered = (buffer[prev] < triggerLevel && buffer[writePos] >= triggerLevel);
            else
                triggered = (buffer[prev] > triggerLevel && buffer[writePos] <= triggerLevel);
        }
    }
    repaint();
}

void PDScope::onNanoDisplay()
{
    const float scaleFactor = getTopLevelWidget()->getScaleFactor();
    const DGL::Rectangle<float> b(0.0f, 0.0f, getWidth(), getHeight());
    NVGcontext* nvg = getContext();

    // Background
    drawRoundedRect(nvg, b.getX(), b.getY(), b.getWidth(), b.getHeight(),
                    bgColor, Colors::outColor, Corners::objectCornerRadius * scaleFactor);

    const DGL::Rectangle<float> area = reduceRectangle(b, 1.5f * scaleFactor);
    const float centerY = area.getY() + area.getHeight() / 2.0f;

    // Draw center line
    nvgBeginPath(nvg);
    nvgMoveTo(nvg, area.getX(), centerY);
    nvgLineTo(nvg, area.getX() + area.getWidth(), centerY);
    nvgStrokeColor(nvg, nvgRGBA(Colors::ioColor.r * 255, Colors::ioColor.g * 255,
                                 Colors::ioColor.b * 255, 0.3f * 255));
    nvgStrokeWidth(nvg, 0.5f * scaleFactor);
    nvgStroke(nvg);

    // Draw grid lines
    for (int i = 1; i < 4; ++i)
    {
        float t = i / 4.0f;
        nvgBeginPath(nvg);
        nvgMoveTo(nvg, area.getX(), area.getY() + area.getHeight() * t);
        nvgLineTo(nvg, area.getX() + area.getWidth(), area.getY() + area.getHeight() * t);
        nvgStroke(nvg);
    }

    if (bufferSize < 2)
        return;

    // Draw trace
    nvgBeginPath(nvg);

    uint32_t startRead;
    if (triggered)
    {
        startRead = writePos;
        triggered = false;
    }
    else
    {
        startRead = (writePos + bufferSize - bufferSize / 4) % bufferSize;
    }

    for (uint32_t i = 0; i < bufferSize; ++i)
    {
        uint32_t idx = (startRead + i) % bufferSize;
        float x = area.getX() + (float)i / (float)(bufferSize - 1) * area.getWidth();
        float y = centerY - buffer[idx] * area.getHeight() * 0.45f;

        if (i == 0)
            nvgMoveTo(nvg, x, y);
        else
            nvgLineTo(nvg, x, y);
    }

    nvgStrokeColor(nvg, traceColor);
    nvgStrokeWidth(nvg, 1.5f * scaleFactor);
    nvgStroke(nvg);
}

void PDScope::setColors(NVGcolor bgColor, NVGcolor traceColor)
{
    this->bgColor = bgColor;
    this->traceColor = traceColor;
}

void PDScope::setTriggerLevel(float level)
{
    triggerLevel = jmax(-1.0f, jmin(1.0f, level));
}

void PDScope::setTriggerMode(bool rising)
{
    triggerRising = rising;
}

END_NAMESPACE_DISTRHO
