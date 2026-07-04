/* 
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
 */

#include "nanovg.h"

#include "Common.hpp"
#include "Image.hpp"

START_NAMESPACE_DISTRHO

PDImage::PDImage(NanoSubWidget* parent)
    : PDWidget(parent),
      bgColor(Colors::bgColor),
      borderColor_(Colors::outColor)
{
}

PDImage::~PDImage()
{
    NVGcontext* nvg = getContext();
    if (nvg != nullptr && imageHandle >= 0)
        nvgDeleteImage(nvg, imageHandle);
}

void PDImage::setImageData(const unsigned char* data, uint32_t width, uint32_t height)
{
    NVGcontext* nvg = getContext();
    if (nvg == nullptr)
        return;

    // Free previous image if exists
    if (imageHandle >= 0)
    {
        nvgDeleteImage(nvg, imageHandle);
        imageHandle = -1;
    }

    if (data != nullptr && width > 0 && height > 0)
    {
        imageHandle = nvgCreateImageRGBA(nvg, width, height, 0, data);
        imgWidth = width;
        imgHeight = height;
    }
    repaint();
}

void PDImage::onNanoDisplay()
{
    const float scaleFactor = getTopLevelWidget()->getScaleFactor();
    const DGL::Rectangle<float> b(0.0f, 0.0f, getWidth(), getHeight());
    NVGcontext* nvg = getContext();

    // Background
    drawRoundedRect(nvg, b.getX(), b.getY(), b.getWidth(), b.getHeight(),
                    bgColor, borderColor_, Corners::objectCornerRadius * scaleFactor);

    // Draw image if available
    if (imageHandle >= 0)
    {
        const DGL::Rectangle<float> area = reduceRectangle(b, 1.0f * scaleFactor);

        NVGpaint imgPaint = nvgImagePattern(nvg,
            area.getX(), area.getY(),
            area.getWidth(), area.getHeight(),
            0.0f, imageHandle, 1.0f);

        nvgBeginPath(nvg);
        nvgRoundedRect(nvg, area.getX(), area.getY(),
                       area.getWidth(), area.getHeight(),
                       (Corners::objectCornerRadius * scaleFactor) / 2.0f);
        nvgFillPaint(nvg, imgPaint);
        nvgFill(nvg);
    }
}

void PDImage::setBackgroundColor(NVGcolor bgColor)
{
    this->bgColor = bgColor;
}

void PDImage::setBorderColor(NVGcolor borderColor)
{
    this->borderColor_ = borderColor;
}

END_NAMESPACE_DISTRHO
