/*
 * Copyright (C) 2026 Wasted Audio
 * SPDX-License-Identifier: ISC
 */

#include "nanovg.h"

#include "Canvas.hpp"
#include "Common.hpp"
#include "Label.hpp"

START_NAMESPACE_DISTRHO

PDCanvas::PDCanvas(NanoSubWidget* parent)
    : NanoWidget(parent)
{

}

PDCanvas::~PDCanvas()
{
    NVGcontext* nvg = getContext();
    if (nvg != nullptr && imageHandle >= 0)
        nvgDeleteImage(nvg, imageHandle);
}

void PDCanvas::onNanoDisplay()
{
    const float scaleFactor = getTopLevelWidget()->getScaleFactor();
    const DGL::Rectangle<float> b(0.0f, 0.0f, getWidth(), getHeight());

    NVGcontext* nvg = getContext();

    drawRoundedRect(nvg, b.getX(), b.getY(), b.getWidth(), b.getHeight(), bgColor, bgColor, Corners::objectCornerRadius * scaleFactor);

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
                       Corners::objectCornerRadius * scaleFactor);
        nvgFillPaint(nvg, imgPaint);
        nvgFill(nvg);
    }
}

void PDCanvas::setColors(NVGcolor bgColor)
{
    this->bgColor = bgColor;
}

void PDCanvas::setLabel(std::string text, NVGcolor textColor, int x, int y, int size)
{
    this->label = new PDLabel(this);
    this->label->setText(text);
    this->label->setColors(textColor);
    this->label->setAbsolutePos(x, (y - size / 2));
    this->label->setSize(size * text.length(), size);
}

void PDCanvas::updateLabelText(std::string text)
{
    if (this->label != nullptr)
        this->label->setText(text);
}

void PDCanvas::setImageData(const unsigned char* data, uint32_t width, uint32_t height)
{
    NVGcontext* nvg = getContext();
    if (nvg == nullptr)
        return;

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

void PDCanvas::setImageFromMemory(const unsigned char* fileData, uint32_t fileSize)
{
    NVGcontext* nvg = getContext();
    if (nvg == nullptr)
        return;

    if (imageHandle >= 0)
    {
        nvgDeleteImage(nvg, imageHandle);
        imageHandle = -1;
    }

    if (fileData != nullptr && fileSize > 0)
    {
        imageHandle = nvgCreateImageMem(nvg, 0, fileData, fileSize);
        if (imageHandle >= 0)
            nvgImageSize(nvg, imageHandle, (int*)&imgWidth, (int*)&imgHeight);
    }
    repaint();
}

END_NAMESPACE_DISTRHO
